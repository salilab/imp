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

#if IMP_HAS_DEPRECATED
//! Don't use
#define IMP_CLUSTERING(Name)                            \
  IMP_PRAGMA(message("Don't use the macro, simple declare the methods.")) \
  unsigned int get_number_of_clusters() const;          \
  const Ints& get_cluster(unsigned int i) const;        \
  int get_cluster_representative(unsigned int i) const; \
  IMP_OBJECT(Name)
#endif

#endif /* IMPSTATISTICS_CLUSTERING_MACROS_H */
