/**
 *  \file statistics/macros.h    \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_MACROS_H
#define IMPSTATISTICS_MACROS_H

#include <IMP/macros.h>


//! Define things needed for a Embedding
/** In addition to what is done by IMP_OBJECT() it declares
    - IMP::statistics::Distance::get_embedding()
*/
#define IMP_EMBEDDING(Name, version_info)                          \
  Floats get_embedding(unsigned int i) const;                      \
  IMP_OBJECT(Name, version_info)


//! Define the methods needed by Clustering
/** In addition to what is defined/declared by IMP_OBJECT() it declares
    - IMP::statistics::Clustering::get_number_of_clusters()
    - IMP::statistics::Clustering::get_cluster()
    - IMP::statistics::Clustering::get_cluster_representative()
*/
#define IMP_CLUSTERING(Name, version_info)                      \
  unsigned int get_number_of_clusters() const;                  \
  const Ints& get_cluster(unsigned int i) const;                \
  int get_cluster_representative(unsigned int i) const;         \
  IMP_OBJECT(Name, version_info)

#endif  /* IMPSTATISTICS_MACROS_H */
