/**
 *  \file IMP/statistics/ChiSquareMetric.h
 *  \brief Chi2 Metric
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPSTATISTICS_CHI_SQUARE_METRIC_H
#define IMPSTATISTICS_CHI_SQUARE_METRIC_H

#include <IMP/statistics/statistics_config.h>
#include <IMP/statistics/Metric.h>
#include <IMP/object_macros.h>
#include <IMP/types.h>
#include <IMP/Pointer.h>

IMPSTATISTICS_BEGIN_NAMESPACE

//! Compute the distance between two configurations using chi2
/** Each configurations is defined by a list of data points and
 *  a list of standard deviations. The distance between the two
 *  configurations is defined as the chi2.
 */
class IMPSTATISTICSEXPORT ChiSquareMetric : public Metric {

  //! list of weights
  Floats weight_;
  //! list of experimental data points
  Floats data_exp_;
  //! list of normalizations
  Floats norm_;
  //! type of constructor
  int constr_type_;
  //! list of lists of data points
  std::vector<Floats> datas_;
  //! list of lists of experimental data points
  std::vector<Floats> stddev_;

  //! get chi2 between two configurations
  double get_chisquare(unsigned i, unsigned j) const;
  //! get scalar chi2 between two configurations
  double get_scalarchisquare(unsigned i, unsigned j) const;

 public:
  ChiSquareMetric(Floats data_exp, int constr_type = 0);
  //! add a configuration, defined by a list of data points,
  //! a list of standard deviations, and one weight.
  void add_configuration(Floats data, Floats stddev, double weight = 1.0);
  //! get the list of data points for the i-th configuration
  Floats get_data(unsigned i) const;
  //! get the list of standard deviations for the i-th configuration
  Floats get_standard_deviation(unsigned i) const;
  //! get the weight for the i-th configuration
  double do_get_weight(unsigned i) const IMP_OVERRIDE;
  //! get chi2 from experimental data for the i-th configuration
  double get_chi_square_experimental(unsigned i) const;

  double get_distance(unsigned int i, unsigned int j) const IMP_OVERRIDE;
  unsigned int get_number_of_items() const IMP_OVERRIDE;

  IMP_OBJECT_METHODS(ChiSquareMetric);
};

IMPSTATISTICS_END_NAMESPACE

#endif /* IMPSTATISTICS_CHI_SQUARE_METRIC_H */
