/**
 *  \file IMP/statistics/Metric.h
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_METRIC_H
#define IMPSTATISTICS_METRIC_H

#include <IMP/statistics/statistics_config.h>
#include <IMP/Object.h>
#include <IMP/object_macros.h>
#include <IMP/ref_counted_macros.h>

IMPSTATISTICS_BEGIN_NAMESPACE

//! Store data to be clustered for distance metric based algorithms
/** Metric clustering needs a way of computing the
    distances between the things being clustered. Classes that
    implement this abstract base class provide a way of getting the
    distance between any two items.
 */
class IMPSTATISTICSEXPORT Metric : public IMP::Object {
 protected:
  Metric(std::string name);
  //! By default return 1.0
  virtual double do_get_weight(unsigned int i) const {
    IMP_UNUSED(i);
    return 1.0;
  }

 public:
  virtual double get_distance(unsigned int i, unsigned int j) const = 0;
  //! Return a weight for the point.
  double get_weight(unsigned int i) const { return do_get_weight(i); }
  virtual unsigned int get_number_of_items() const = 0;
  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Metric);
};

IMP_OBJECTS(Metric, Metrics);

IMPSTATISTICS_END_NAMESPACE

#endif /* IMPSTATISTICS_METRIC_H */
