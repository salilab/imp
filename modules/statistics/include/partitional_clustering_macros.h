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

IMPSTATISTICS_DEPRECATED_HEADER(2.1, "It is not needed any more")

//! Simply declare the needed methods directly
#define IMP_PARTITIONAL_CLUSTERING(Name)                                \
  IMP_DEPRECATED_MACRO(2.1, "Don't use the macro, simple declare the methods.")\
  unsigned int get_number_of_clusters() const IMP_OVERRIDE;             \
  const Ints& get_cluster(unsigned int i) const IMP_OVERRIDE;           \
  int get_cluster_representative(unsigned int i) const IMP_OVERRIDE;    \
  IMP_OBJECT_NO_WARNING(Name)

#endif /* IMPSTATISTICS_PARTITIONAL_CLUSTERING_MACROS_H */
