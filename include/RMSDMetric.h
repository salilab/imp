/**
 *  \file RMSDMetric.h
 *  \brief Distance RMSD Metric
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_RMSD_METRIC_H
#define IMPMEMBRANE_RMSD_METRIC_H

#include "membrane_config.h"
#include <IMP/statistics.h>
#include <IMP/algebra.h>

IMPMEMBRANE_BEGIN_NAMESPACE

/** Compute the RMSD between two sets of particles in two configurations.
 */
class IMPMEMBRANEEXPORT RMSDMetric: public statistics::Metric {

  Particles ps_;
  Floats weight_;
  std::vector< algebra::Vector3Ds > coords_;

  double get_rmsd(algebra::Vector3Ds v0, algebra::Vector3Ds v1) const;

 public:
  RMSDMetric(Particles ps);
  void add_configuration(double weight=1.0);
  Float get_weight(unsigned i);

  //IMP_METRIC(RMSDMetric);
  double get_distance(unsigned int i, unsigned int j) const IMP_OVERRIDE;
  unsigned int get_number_of_items() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RMSDMetric);
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_RMSD_METRIC_H */
