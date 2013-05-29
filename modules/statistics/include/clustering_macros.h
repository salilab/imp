/**
 *  \file IMP/statistics/clustering_macros.h    \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_CLUSTERING_MACROS_H
#define IMPSTATISTICS_CLUSTERING_MACROS_H

#include "embedding_macros.h"
#include <IMP/base/object_macros.h>

//! Define things needed for a Metric
/** In addition to what is done by IMP_OBJECT() it declares
    - IMP::statistics::Distance::get_embedding()
*/
#define IMP_METRIC(Name)                                     \
  double get_distance(unsigned int i, unsigned int j) const; \
  unsigned int get_number_of_items() const;                  \
  IMP_OBJECT(Name)

//! Define the methods needed by Clustering
/** In addition to what is defined/declared by IMP_OBJECT() it declares
    - IMP::statistics::Clustering::get_number_of_clusters()
    - IMP::statistics::Clustering::get_cluster()
    - IMP::statistics::Clustering::get_cluster_representative()
*/
#define IMP_CLUSTERING(Name)                            \
  unsigned int get_number_of_clusters() const;          \
  const Ints& get_cluster(unsigned int i) const;        \
  int get_cluster_representative(unsigned int i) const; \
  IMP_OBJECT(Name)

#endif /* IMPSTATISTICS_CLUSTERING_MACROS_H */
