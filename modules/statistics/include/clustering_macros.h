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

IMPSTATISTICS_DEPRECATED_HEADER(2.1, "It is not needed any more")

//! \deprecated_at{2.1} Do not use any more
#define IMP_CLUSTERING(Name)                                                \
  IMP_DEPRECATED_MACROS(                                                    \
      2.1, "Don't use the macro, simple declare the methods.") unsigned int \
      get_number_of_clusters() const;                                       \
  const Ints& get_cluster(unsigned int i) const;                            \
  int get_cluster_representative(unsigned int i) const;                     \
  IMP_OBJECT_NO_WARNING(Name)

#endif /* IMPSTATISTICS_CLUSTERING_MACROS_H */
