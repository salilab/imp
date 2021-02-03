/**
 *  \file IMP/statistics/metrics.h
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_METRICS_H
#define IMPSTATISTICS_METRICS_H

#include <IMP/statistics/statistics_config.h>
#include "Metric.h"
#include "Embedding.h"
#include <IMP/object_macros.h>
#include <IMP/Pointer.h>
#include <IMP/ConfigurationSet.h>
#include <IMP/SingletonContainer.h>

IMPSTATISTICS_BEGIN_NAMESPACE

/** Apply a Euclidean metric to an Embedding. */
class IMPSTATISTICSEXPORT EuclideanMetric : public Metric {
  IMP::PointerMember<Embedding> em_;

 public:
  EuclideanMetric(Embedding *em);
  double get_distance(unsigned int i, unsigned int j) const IMP_OVERRIDE;
  unsigned int get_number_of_items() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(EuclideanMetric);
};

/** Compute the RMSD between specified sets of particles
    in pairs of configurations, within a configuration set
 */
class IMPSTATISTICSEXPORT ConfigurationSetRMSDMetric : public Metric {
  IMP::PointerMember<ConfigurationSet> cs_;
  IMP::PointerMember<SingletonContainer> sc_;
  bool align_;

 public:
  /**
     Constructor for creating a metric that computes RMSD between
     pairs of configurations in a configuration set, using joint particles
     specified in a singleton container

     @param cs the set of configurations
     @param sc the particles used for RMSD calculation between each pair of
               configuration
     @param align whether to align pair of configurations prior to RMSD
            calculations
   */
  ConfigurationSetRMSDMetric(ConfigurationSet *cs, SingletonContainer *sc,
                             bool align = false);
  double get_distance(unsigned int i, unsigned int j) const IMP_OVERRIDE;
  unsigned int get_number_of_items() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(ConfigurationSetRMSDMetric);
};

IMPSTATISTICS_END_NAMESPACE

#endif /* IMPSTATISTICS_METRICS_H */
