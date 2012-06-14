/**
 *  \file metrics.h
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPSTATISTICS_METRICS_H
#define IMPSTATISTICS_METRICS_H

#include "statistics_config.h"
#include "Metric.h"
#include "Embedding.h"
#include "metric_macros.h"
#include <IMP/base/Pointer.h>
#include <IMP/ConfigurationSet.h>
#include <IMP/SingletonContainer.h>

IMPSTATISTICS_BEGIN_NAMESPACE

class IMPSTATISTICSEXPORT EuclideanMetric: public Metric {
  IMP::base::OwnerPointer<Embedding> em_;
public:
  EuclideanMetric(Embedding *em);
  IMP_METRIC(EuclideanMetric);
};


/** Compute the RMSD between two sets of particles in two configurations.
 */
class IMPSTATISTICSEXPORT ConfigurationSetRMSDMetric: public Metric {
  IMP::OwnerPointer<ConfigurationSet> cs_;
  IMP::OwnerPointer<SingletonContainer> sc_;
  bool align_;
 public:
  ConfigurationSetRMSDMetric(ConfigurationSet *cs,
                              SingletonContainer *sc,
                              bool align=false);
  IMP_METRIC(ConfigurationSetRMSDMetric);
};


IMPSTATISTICS_END_NAMESPACE

#endif /* IMPSTATISTICS_METRICS_H */
