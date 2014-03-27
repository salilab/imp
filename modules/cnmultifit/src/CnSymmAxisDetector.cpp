/**
 *  \file CnSymmAxisDetector.cpp
 *  \brief Detect cn symmetry in proteins and density maps.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/cnmultifit/CnSymmAxisDetector.h>
#include <IMP/em/converters.h>
#include <IMP/statistics/Histogram.h>
#include <algorithm>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/algebra/constants.h>

IMPCNMULTIFIT_BEGIN_NAMESPACE

namespace {

class DensityHistogram {
  double start_, end_, interval_size_;
  Ints freq_;
public:
  DensityHistogram(double start, double end, unsigned int num_bins)
                 : start_(start), end_(end) {
    interval_size_ = (end - start) / num_bins;
    freq_.insert(freq_.end(), num_bins, 0);
  }

  //! Increase the count for the bin that holds a
  // value that is in range for this histogram.
  void add(double x) {
    IMP_INTERNAL_CHECK(!(x < start_ || x > end_),
                     "the value being added to the "
                         << "histogram " << x << " is out of range [" << start_
                         << "," << end_ << "]" << std::endl);
    const unsigned int i =
      (static_cast<unsigned int>((x - start_) / interval_size_));
    freq_[i]++;
  }

  unsigned get_total_count() const {
    return std::accumulate(freq_.begin(), freq_.end(), 0);
  }

  //! Get the lowest value for which X% of the histogram data is lower
  double get_top(double percentage) const {
    IMP_INTERNAL_CHECK(!((percentage < 0.) || (percentage > 1.)),
                       "The input number is not a percentage\n");
    double stop_count = get_total_count() * percentage;
    int partial_count = 0;
    for (unsigned int i = 0; i < freq_.size(); i++) {
      partial_count += freq_[i];
      if (partial_count > stop_count) {
        return start_ + i * interval_size_;
      }
    }
    return end_;
  }
};

double get_top_density_value(em::DensityMap *dmap, float density_threshold,
                             float top_p) {
  DensityHistogram dhist(density_threshold - em::EPS,
                         dmap->get_max_value() + .1, 100);
  for (long v_ind = 0; v_ind < dmap->get_number_of_voxels(); v_ind++) {
    if (dmap->get_value(v_ind) > density_threshold) {
      dhist.add(dmap->get_value(v_ind));
    }
  }
  return dhist.get_top(top_p);
}

}

CnSymmAxisDetector::CnSymmAxisDetector(int symm_deg, em::DensityMap *dmap,
                                       float density_threshold, float top_p)
    : dmap_(dmap), symm_deg_(symm_deg) {
  double top_p_den_val = get_top_density_value(dmap_, density_threshold, top_p);
  vecs_ = em::density2vectors(dmap_, top_p_den_val);
  //  std::cout<<"Taking "<<vecs_.size()<<" vectors with density higher than :"
  //  <<top_p_den_val<<std::endl;

  // calculate  pca
  pca_ = algebra::get_principal_components(vecs_);
  // calculate transformation from the native axes system to the one
  // defined by the pca
  from_native_ = algebra::get_rotation_from_x_y_axes(
      pca_.get_principal_component(0), pca_.get_principal_component(1));
  to_native_ = from_native_.get_inverse();
}

CnSymmAxisDetector::CnSymmAxisDetector(int symm_deg,
                                       const atom::Hierarchies &mhs)
    : symm_deg_(symm_deg) {
  // create a density map
  kernel::Particles ps;
  for (atom::Hierarchies::const_iterator it = mhs.begin(); it != mhs.end();
       it++) {
    kernel::Particles temp_ps = core::get_leaves(*it);
    ps.insert(ps.end(), temp_ps.begin(), temp_ps.end());
  }
  // TODO - check the number of particles!!
  IMP_NEW(em::SampledDensityMap, sampled_dmap, (ps, 3., 1.));
  sampled_dmap->resample();
  sampled_dmap->calcRMS();
  dmap_ = new em::DensityMap(*(sampled_dmap->get_header()));
  dmap_->copy_map(sampled_dmap);
  double top_20_den_val = get_top_density_value(dmap_,
                                          dmap_->get_header()->dmin, 0.8);
  vecs_ = em::density2vectors(dmap_, top_20_den_val);
  // calculate  pca
  pca_ = algebra::get_principal_components(vecs_);
  // calculate transformation from the native axes system to the one
  // defined by the pca
  from_native_ = algebra::get_rotation_from_x_y_axes(
      pca_.get_principal_component(0), pca_.get_principal_component(1));
  to_native_ = from_native_.get_inverse();
  sampled_dmap = nullptr;
}

int CnSymmAxisDetector::get_symmetry_axis_index() const {
  int symm_ind = 0;
  float symm_s, tmp_s;
  symm_s = calc_symm_score(0);
  tmp_s = calc_symm_score(1);
  if (tmp_s < symm_s) {
    symm_s = tmp_s;
    symm_ind = 1;
  }
  tmp_s = calc_symm_score(2);
  if (tmp_s < symm_s) {
    symm_s = tmp_s;
    symm_ind = 2;
  }
  return symm_ind;
}
algebra::Vector3D CnSymmAxisDetector::get_symmetry_axis() const {
  return pca_.get_principal_component(get_symmetry_axis_index());
}

// only rotation
// axis_symm will be used as z-axis and the center of mass is assumed to be 000
float CnSymmAxisDetector::calc_symm_score(int symm_axis_ind) const {
  int ind1, ind2;
  if (symm_axis_ind == 0) {
    ind1 = 1;
    ind2 = 2;
  } else if (symm_axis_ind == 1) {
    ind1 = 0;
    ind2 = 2;
  } else {
    ind1 = 0;
    ind2 = 1;
  }
  // symm axis is in the PCA reference frame.
  // we want to move it to the native resference frame
  float angle = 2 * algebra::PI / symm_deg_;
  algebra::Transformation3D from_native_xy_to_pca_xy =
      algebra::get_rotation_from_x_y_axes(pca_.get_principal_component(ind1),
                                          pca_.get_principal_component(ind2));
  // translate the PCA origin to 000
  algebra::Transformation3D pca_to_native_translation =
      algebra::Transformation3D(algebra::get_identity_rotation_3d(),
                                -pca_.get_centroid());
  // build a transformation such that the symm axis is on Z
  algebra::Transformation3D from_pca_to_native =
      from_native_xy_to_pca_xy.get_inverse() * pca_to_native_translation;
  // build the symmetric transformation (move to native, rotate around Z
  //, and move back to PCA reference frame)
  algebra::Transformation3D symmetric_trans =
      from_pca_to_native.get_inverse() *
      algebra::Transformation3D(algebra::get_rotation_about_axis(
                                    algebra::Vector3D(0., 0., 1.), angle),
                                algebra::Vector3D(0., 0., 0.)) *
      from_pca_to_native;

  float avg_std = 0, max_std = -9999, min_std = 9999;
  long avg_std_count = 0;
  for (algebra::Vector3Ds::const_iterator it = vecs_.begin(); it != vecs_.end();
       it++) {
    algebra::Vector3D location = *it;
    std::vector<float> dens_vals;
    for (int symm = 0; symm < symm_deg_; symm++) {
      location = symmetric_trans.get_transformed(location);
      if (dmap_->is_part_of_volume(location)) {
        dens_vals.push_back(dmap_->get_value(location));
      }
    }
    if (((int)dens_vals.size()) != symm_deg_) {
      continue;
    }
    // average
    float avg_density =
        std::accumulate(dens_vals.begin(), dens_vals.end(), 0.) /
        dens_vals.size();
    // variance
    float var = 0.0;
    for (unsigned int i = 0; i < dens_vals.size(); i++) {
      var += (dens_vals[i] - avg_density) * (dens_vals[i] - avg_density);
    }
    var /= dens_vals.size();
    avg_std_count++;
    avg_std += sqrt(var);

    min_std = (min_std > sqrt(var)) ? sqrt(var) : min_std;
    max_std = (max_std < sqrt(var)) ? sqrt(var) : max_std;
  }  // finish going over the particles

  avg_std /= avg_std_count;
  IMP_LOG_TERSE("In symmetry axis voting:" << std::endl);
  IMP_LOG_TERSE("average STD from " << avg_std_count
                                    << " particles: " << avg_std << std::endl);
  IMP_LOG_TERSE("Minimum STD " << min_std << std::endl);
  IMP_LOG_TERSE("Maximum STD " << max_std << std::endl);
  return avg_std;
}

Float CnSymmAxisDetector::symm_avg(const algebra::Vector3D &start_p,
                                   const algebra::Vector3D &symm_vec) const {
  Float val = 0.;
  algebra::Vector3D p = start_p;
  algebra::Transformation3D symm_op;
  for (int i = 0; i < symm_deg_; i++) {
    val += dmap_->get_value(p);
    p = symm_op.get_transformed(p);
  }
  return val / symm_deg_;
}

IMPCNMULTIFIT_END_NAMESPACE
