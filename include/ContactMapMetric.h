/**
 *  \file ContactMapMetric.h
 *  \brief Contact Map Metric
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_CONTACT_MAP_METRIC_H
#define IMPMEMBRANE_CONTACT_MAP_METRIC_H

#include "membrane_config.h"
#include <IMP/statistics.h>
#include <IMP/algebra.h>

IMPMEMBRANE_BEGIN_NAMESPACE

/** Compute the RMSD between two sets of particles in two configurations.
 */
class IMPMEMBRANEEXPORT ContactMapMetric: public statistics::Metric {

  Particles ps_;
  double r0_;
  int nn_;
  int mm_;
  Floats weight_;
  std::vector<Floats> matrices_;

  double get_rmsd(Floats m0, Floats m1) const;
  Floats get_contact_map(algebra::Vector3Ds coords) const;

 public:
  ContactMapMetric(Particles ps, double r0, int nn, int mm);

  void add_configuration(double weight=1.0);
  void add_map(Floats matrix, double weight=1.0);

  Float  get_weight(unsigned i) const;
  Floats get_item(unsigned i) const;

  //IMP_METRIC(ContactMapMetric);
  double get_distance(unsigned int i, unsigned int j) const IMP_OVERRIDE;
  unsigned int get_number_of_items() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(ContactMapMetric);
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_CONTACT_MAP_METRIC_H */
