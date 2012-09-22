/**
 *  \file ChiSquareMetric.h
 *  \brief Contact Map Metric
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_CHI_SQUARE_METRIC_H
#define IMPMEMBRANE_CHI_SQUARE_METRIC_H

#include "membrane_config.h"
#include <IMP/statistics.h>
#include <IMP/algebra.h>

IMPMEMBRANE_BEGIN_NAMESPACE

/** Compute the RMSD between two sets of particles in two configurations.
 */
class IMPMEMBRANEEXPORT ChiSquareMetric: public statistics::Metric {

  Floats weight_;
  std::vector<Floats> nus_;
  std::vector<Floats> stddev_;

  double get_chisquare(Floats nu0, Floats stddev0,
                       Floats nu1, Floats stddev1) const;

 public:
  ChiSquareMetric();

  void add_configuration(Floats nu, Floats stddev, double weight=1.0);

  Floats get_nu(unsigned i) const;
  Floats get_stddev(unsigned i) const;
  Float  get_weight(unsigned i) const;

  IMP_METRIC(ChiSquareMetric);
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_CHI_SQUARE_METRIC_H */
