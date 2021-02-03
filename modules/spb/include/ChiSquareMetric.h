/**
 *  \file IMP/spb/ChiSquareMetric.h
 *  \brief Contact Map Metric
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPSPB_CHI_SQUARE_METRIC_H
#define IMPSPB_CHI_SQUARE_METRIC_H

#include <IMP/algebra.h>
#include <IMP/statistics.h>
#include "spb_config.h"

IMPSPB_BEGIN_NAMESPACE

/** Compute the RMSD between two sets of particles in two configurations.
 */
class IMPSPBEXPORT ChiSquareMetric : public statistics::Metric {
  Floats weight_;
  Floats nu_exp_;
  Floats norm_;
  int constr_type_;

  std::vector<Floats> nus_;
  std::vector<Floats> stddev_;

  double get_chisquare(unsigned i, unsigned j) const;

  double get_scalarchisquare(unsigned i, unsigned j) const;

 public:
  // double constructor
  ChiSquareMetric(Floats nu_exp, int constr_type = 0);

  void add_configuration(Floats nu, Floats stddev, double weight = 1.0);

  Floats get_nu(unsigned i) const;
  Floats get_stddev(unsigned i) const;
  Float get_weight(unsigned i) const;

  double get_chisquare_exp(unsigned i) const;

  double get_distance(unsigned int i, unsigned int j) const IMP_OVERRIDE;
  unsigned int get_number_of_items() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(ChiSquareMetric);

  // IMP_METRIC(ChiSquareMetric);
};

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_CHI_SQUARE_METRIC_H */
