/**
 *  \file IMP/statistics/metrics.h
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_METRICS_H
#define IMPSTATISTICS_METRICS_H

#include <IMP/statistics/statistics_config.h>
#include "Metric.h"
#include "Embedding.h"
#include "metric_macros.h"
#include <IMP/base/Pointer.h>
#include <IMP/ConfigurationSet.h>
#include <IMP/SingletonContainer.h>

IMPSTATISTICS_BEGIN_NAMESPACE

class IMPSTATISTICSEXPORT EuclideanMetric : public Metric {
  IMP::base::OwnerPointer<Embedding> em_;

 public:
  EuclideanMetric(Embedding *em);
  IMP_METRIC(EuclideanMetric);
};

/** Compute the RMSD between specified sets of particles
    in pairs of configurations, within a configuration set
 */
class IMPSTATISTICSEXPORT ConfigurationSetRMSDMetric : public Metric {
  IMP::OwnerPointer<ConfigurationSet> cs_;
  IMP::OwnerPointer<SingletonContainer> sc_;
  bool align_;

 public:
  /**
     Constructor for creating a metric that computes RMSD between
     pairs of configurations in a configuration set, using joint particles
     specified in a singleton continer

     @param cs the set of configurations
     @param sc the particles used for RMSD calculation between each pair of
               configuration
     @param align whether to align pair of configurations prior to RMSD
            calculations
   */
  ConfigurationSetRMSDMetric(ConfigurationSet *cs, SingletonContainer *sc,
                             bool align = false);
  IMP_METRIC(ConfigurationSetRMSDMetric);
};

IMPSTATISTICS_END_NAMESPACE

#endif /* IMPSTATISTICS_METRICS_H */
