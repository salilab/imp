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

#if IMP_HAS_DEPRECATED
//! Simply declare the needed methods directly
#define IMP_PARTITIONAL_CLUSTERING(Name)                                \
  IMP_PRAGMA(message("Don't use the macro, simple declare the methods.")) \
  unsigned int get_number_of_clusters() const IMP_OVERRIDE;             \
  const Ints& get_cluster(unsigned int i) const IMP_OVERRIDE;           \
  int get_cluster_representative(unsigned int i) const IMP_OVERRIDE;    \
  IMP_OBJECT(Name)
#endif

#endif /* IMPSTATISTICS_PARTITIONAL_CLUSTERING_MACROS_H */
