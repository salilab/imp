/**
 *  \file distance_clustering.h
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPSTATISTICS_DISTANCE_CLUSTERING_H
#define IMPSTATISTICS_DISTANCE_CLUSTERING_H

#include "statistics_config.h"
#include "statistics_macros.h"
#include "PartitionalClustering.h"
#include <IMP/algebra/VectorD.h>

IMPSTATISTICS_BEGIN_NAMESPACE

//! Compute a distance between two elements to be clustered
/** Distance-based clustering needs a way of computing the
    distances between the things being clustered.
 */
class IMPSTATISTICSEXPORT Distance: public Object {
public:
  Distance(std::string name);
  virtual double get_distance(unsigned int i,
                              unsigned int j) const =0;
  virtual unsigned int get_number_of_items() const=0;
  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Distance);
};

IMP_OBJECTS(Distance, Distances);

/** Cluster by repeatedly removing edges which have lots
    of shortest paths passing through them. The process can
    be terminated either when there are a set number of
    connected components.
 */
IMPSTATISTICSEXPORT
PartitionalClustering *get_centrality_clustering(Distance *d,
                                                 double far,
                                                 int k);

IMPSTATISTICS_END_NAMESPACE

#endif /* IMPSTATISTICS_DISTANCE_CLUSTERING_H */
