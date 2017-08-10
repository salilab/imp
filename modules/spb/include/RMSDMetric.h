/**
 *  \file IMP/spb/RMSDMetric.h
 *  \brief Distance RMSD Metric
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 */

#ifndef IMPSPB_RMSD_METRIC_H
#define IMPSPB_RMSD_METRIC_H

#include <IMP/algebra.h>
#include <IMP/statistics.h>
#include "spb_config.h"

IMPSPB_BEGIN_NAMESPACE

/** Compute the RMSD between two sets of particles in two configurations.
 */
class IMPSPBEXPORT RMSDMetric : public statistics::Metric {
  Particles ps_;
  Floats weight_;
  std::vector<algebra::Vector3Ds> coords_;

  double get_rmsd(algebra::Vector3Ds v0, algebra::Vector3Ds v1) const;

 public:
  RMSDMetric(Particles ps);
  void add_configuration(double weight = 1.0);
  Float get_weight(unsigned i);

  // IMP_METRIC(RMSDMetric);
  double get_distance(unsigned int i, unsigned int j) const IMP_OVERRIDE;
  unsigned int get_number_of_items() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RMSDMetric);
};

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_RMSD_METRIC_H */
