/**
 *  \file metric_clustering.h
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPSTATISTICS_METRIC_CLUSTERING_H
#define IMPSTATISTICS_METRIC_CLUSTERING_H

#include "statistics_config.h"
#include "statistics_macros.h"
#include "PartitionalClustering.h"
#include "point_clustering.h"
#include <IMP/algebra/VectorD.h>
#include <IMP/macros.h>
#include <IMP/VectorOfRefCounted.h>
#include <IMP/ConfigurationSet.h>
#include <IMP/SingletonContainer.h>
#include <IMP/internal/OwnerPointer.h>

IMPSTATISTICS_BEGIN_NAMESPACE

//! Compute a distance between two elements to be clustered
/** Metric clustering needs a way of computing the
    distances between the things being clustered.
 */
class IMPSTATISTICSEXPORT Metric: public Object {
public:
  Metric(std::string name);
  virtual double get_distance(unsigned int i,
                              unsigned int j) const =0;
  virtual unsigned int get_number_of_items() const=0;
  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Metric);
};

IMP_OBJECTS(Metric, Metrics);

class IMPSTATISTICSEXPORT EuclideanMetric: public Metric {
  IMP::internal::OwnerPointer<Embedding> em_;
public:
  EuclideanMetric(Embedding *em);
  IMP_METRIC(EuclideanMetric);
};


/** Compute the RMSD between two sets of particles in two configurations.
 */
class IMPSTATISTICSEXPORT ConfigurationSetRMSDMetric: public Metric {
  IMP::internal::OwnerPointer<ConfigurationSet> cs_;
  IMP::internal::OwnerPointer<SingletonContainer> sc_;
  bool align_;
 public:
  ConfigurationSetRMSDMetric(ConfigurationSet *cs,
                              SingletonContainer *sc,
                              bool align=false);
  IMP_METRIC(ConfigurationSetRMSDMetric);
};



/** Cluster by repeatedly removing edges which have lots
    of shortest paths passing through them. The process is
    terminated when there are a set number of
    connected components. Other termination criteria
    can be added if someone proposes them.
 */
IMPSTATISTICSEXPORT
PartitionalClustering *create_centrality_clustering(Metric *d,
                                                 double far,
                                                 int k);

IMPSTATISTICS_END_NAMESPACE

#endif /* IMPSTATISTICS_METRIC_CLUSTERING_H */
