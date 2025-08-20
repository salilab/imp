/**
 *  \file isd/PMFRestraint.cpp
 *  \brief Implementation of PMF restraint
 *
 *  Copyright 2007-2024 IMP Inventors. All rights reserved.
 */

#include <IMP/algebra/VectorD.h>
#include <IMP/container/ClosePairContainer.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/core/XYZ.h>
#include <IMP/isd/PMFRestraint.h>

#include <algorithm>
#include <fstream>
#include <sstream>

IMPISD_BEGIN_NAMESPACE

PMFRestraint::PMFRestraint(Model* m, std::string pmf_file,
                           double distance_cutoff, std::string name)
    : Restraint(m, name), distance_cutoff_(distance_cutoff) {
  load_pmf_from_file(pmf_file);
}

void PMFRestraint::set_containers(IMP::container::ListSingletonContainer* lsc,
                                  IMP::container::ClosePairContainer* cpc) {
  lsc_ = lsc;
  cpc_ = cpc;
}

void PMFRestraint::load_pmf_from_file(std::string filename) {
  pmf_table_.clear();

  std::ifstream file(filename.c_str());
  if (!file) {
    IMP_THROW("Cannot open PMF file: " + filename, IOException);
  }

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#') continue;

    std::stringstream ss(line);
    std::string distance_str, pmf_str;

    if (std::getline(ss, distance_str, ',') && std::getline(ss, pmf_str, ',')) {
      try {
        double distance = std::stod(distance_str);
        double pmf_value = std::stod(pmf_str);
        pmf_table_.push_back(std::make_pair(distance, pmf_value));
      } catch (const std::exception& e) {
        IMP_WARN("Error parsing PMF line: " << line);
      }
    }
  }

  std::sort(pmf_table_.begin(), pmf_table_.end());

  if (!pmf_table_.empty()) {
    min_distance_ = pmf_table_.front().first;
    max_distance_ = pmf_table_.back().first;

    IMP_LOG_TERSE("Loaded " << pmf_table_.size() << " PMF data points from "
                            << filename << " with range [" << min_distance_
                            << ", " << max_distance_ << "]" << std::endl);
  } else {
    IMP_THROW("Empty PMF table loaded from " + filename, ValueException);
  }
}

double PMFRestraint::get_pmf_value(double distance) const {
  if (distance < min_distance_) {
    return 1e4;  // Configurable penalty could be added here
  }

  if (distance > max_distance_) {
    auto it = std::lower_bound(pmf_table_.begin(), pmf_table_.end(),
                               std::make_pair(max_distance_, 0.0));
    if (it != pmf_table_.end()) {
      return it->second;
    }
    IMP_WARN("Distance " << distance << " exceeds maximum PMF distance "
                         << max_distance_ << ". Returning 0.");
    return 0.0;  // Neutral default for large distances
  }

  auto it = std::lower_bound(pmf_table_.begin(), pmf_table_.end(),
                             std::make_pair(distance, 0.0));

  if (it != pmf_table_.end() && it->first == distance) {
    return it->second;
  }

  auto high = it;
  if (high == pmf_table_.begin()) {
    return high->second;
  }

  auto low = high - 1;

  double t = (distance - low->first) / (high->first - low->first);
  return low->second + t * (high->second - low->second);
}

double PMFRestraint::unprotected_evaluate(DerivativeAccumulator* accum) const {
  double score = 0.0;
  if (!cpc_) return score;

  const IMP::ParticleIndexPairs& index_pairs = cpc_->get_indexes();
  // Print to terminal for debugging
  IMP_LOG_TERSE("Evaluating PMFRestraint with " << index_pairs.size()
                                                << " pairs." << std::endl);
  std::cout << "Evaluating PMFRestraint with " << index_pairs.size()
            << " pairs." << std::endl;
  for (const auto& pair : index_pairs) {
    IMP::core::XYZ p1(get_model(), pair[0]);
    IMP::core::XYZ p2(get_model(), pair[1]);
    double distance = IMP::core::get_distance(p1, p2);
    double pmf = get_pmf_value(distance);
    score += pmf;

    if (accum) {
      // Derivative calculation for linear interpolation
      if (distance >= min_distance_ && distance <= max_distance_) {
        auto it = std::lower_bound(pmf_table_.begin(), pmf_table_.end(),
                                   std::make_pair(distance, 0.0));
        auto high = it;
        auto low = high - 1;
        double slope =
            (high->second - low->second) / (high->first - low->first);

        // Unit vector from p1 to p2
        IMP::algebra::Vector3D delta =
            p2.get_coordinates() - p1.get_coordinates();
        double dist = delta.get_magnitude();
        if (dist > 0) {
          IMP::algebra::Vector3D grad = (slope / dist) * delta;
          p1.add_to_derivatives(grad, *accum);
          p2.add_to_derivatives(-grad, *accum);
        }
      }
    }
  }

  return score;
}

ModelObjectsTemp PMFRestraint::do_get_inputs() const {
  if (cpc_) {
    return cpc_->get_inputs();  // Returns particles managed by cpc_
  }
  return ModelObjectsTemp();
}

IMPISD_END_NAMESPACE