/**
 *  \file IMP/statistics/partitional_clustering_macros.h
 *  \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_PARTITIONAL_CLUSTERING_MACROS_H
#define IMPSTATISTICS_PARTITIONAL_CLUSTERING_MACROS_H

#include "PartitionalClustering.h"
#include <IMP/base/object_macros.h>

//! Define the methods needed by Clustering
/** In addition to what is defined/declared by IMP_OBJECT() it declares
    - IMP::statistics::PartitionalClustering::get_number_of_clusters()
    - IMP::statistics::PartitionalClustering::get_cluster()
    - IMP::statistics::PartitionalClustering::get_cluster_representative()
*/
#define IMP_PARTITIONAL_CLUSTERING(Name)                \
  unsigned int get_number_of_clusters() const;          \
  const Ints& get_cluster(unsigned int i) const;        \
  int get_cluster_representative(unsigned int i) const; \
  IMP_OBJECT(Name)

#endif /* IMPSTATISTICS_PARTITIONAL_CLUSTERING_MACROS_H */
