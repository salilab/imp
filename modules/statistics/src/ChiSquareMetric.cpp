/**
 *  \file ChiSquareMetric.cpp
 *  \brief Chi2 Metric
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#include <IMP/statistics/ChiSquareMetric.h>
#include <IMP/algebra.h>
#include <IMP/compatibility/vector_property_map.h>

IMPSTATISTICS_BEGIN_NAMESPACE

ChiSquareMetric::ChiSquareMetric(Floats data_exp, int constr_type)
    : Metric("ChiSquare Metric %1%"),
      data_exp_(data_exp),
      constr_type_(constr_type) {}

void ChiSquareMetric::add_configuration(Floats data, Floats stddev,
                                        double weight) {
  // store weight
  weight_.push_back(weight);
  // if the constructor for normal chi2 is used, store non-normalized data
  if (constr_type_ == 0) {
    datas_.push_back(data);
    // standard deviation
    stddev_.push_back(stddev);
    // else normalize vector (data-data_exp)/stddev
  } else {
    double norm2 = 0.;
    for (unsigned i = 0; i < data.size(); ++i) {
      norm2 += pow((data[i] - data_exp_[i]) / stddev[i], 2);
    }
    norm_.push_back(sqrt(norm2));
    for (unsigned i = 0; i < data.size(); ++i) {
      data[i] = (data[i] - data_exp_[i]) / stddev[i];
    }
    datas_.push_back(data);
  }
  return;
}

double ChiSquareMetric::get_chi_square_experimental(unsigned i) const {
  double chi2 = 0.0;
  if (constr_type_ == 0) {
    for (unsigned k = 0; k < data_exp_.size(); ++k) {
      chi2 += (datas_[i][k] - data_exp_[k]) * (datas_[i][k] - data_exp_[k]) /
              stddev_[i][k] / stddev_[i][k];
    }
  } else {
    for (unsigned k = 0; k < data_exp_.size(); ++k) {
      chi2 += datas_[i][k] * datas_[i][k];
    }
  }
  return chi2 / static_cast<double>(datas_[i].size());
}

double ChiSquareMetric::get_chisquare(unsigned i, unsigned j) const {
  double chi2 = 0.0;
  for (unsigned k = 0; k < datas_[i].size(); ++k) {
    chi2 += (datas_[i][k] - datas_[j][k]) * (datas_[i][k] - datas_[j][k]) /
            (stddev_[i][k] * stddev_[i][k] + stddev_[j][k] * stddev_[j][k]);
  }
  return chi2 / static_cast<double>(datas_[i].size());
}

double ChiSquareMetric::get_scalarchisquare(unsigned i, unsigned j) const {
  double chi2 = 0.0;
  for (unsigned k = 0; k < datas_[i].size(); ++k) {
    chi2 += datas_[i][k] * datas_[j][k] / norm_[i] / norm_[j];
  }
  return acos(chi2);
}

double ChiSquareMetric::get_distance(unsigned i, unsigned j) const {
  if (constr_type_ == 0) {
    return get_chisquare(i, j);
  } else {
    return get_scalarchisquare(i, j);
  }
}

double ChiSquareMetric::do_get_weight(unsigned i) const { return weight_[i]; }

unsigned ChiSquareMetric::get_number_of_items() const { return datas_.size(); }

Floats ChiSquareMetric::get_data(unsigned i) const { return datas_[i]; }

Floats ChiSquareMetric::get_standard_deviation(unsigned i) const {
  return stddev_[i];
}

IMPSTATISTICS_END_NAMESPACE
