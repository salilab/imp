/**
 *  \file PartitionalClustering.h
 *  \brief Compute a distance metric between two points
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPSTATISTICS_PARTITIONAL_CLUSTERING_H
#define IMPSTATISTICS_PARTITIONAL_CLUSTERING_H

#include "config.h"
#include <IMP/macros.h>
#include <IMP/Object.h>

IMPSTATISTICS_BEGIN_NAMESPACE

//! The base class for clusterings of data sets
/** Data items are represented by an index. In a partitional clustering
    each item can only be contained on one cluster. The mapping between
    data items and the information needed by the clustering algorithm is
    maintained by a helper object such as Embedding.
*/
class IMPSTATISTICSEXPORT PartitionalClustering: public Object {
 public:
  PartitionalClustering(std::string name): Object(name){}
  virtual unsigned int get_number_of_clusters() const=0;
  /** Return the list of indexes in the ith cluster.
   */
  virtual const Ints& get_cluster(unsigned int i) const =0;
  /** Return the index of an item that "typifies" the ith cluster.
   */
  virtual int get_cluster_representative(unsigned int i) const=0;
  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(PartitionalClustering);
};

IMPSTATISTICS_END_NAMESPACE
#endif /* IMPSTATISTICS_PARTITIONAL_CLUSTERING_H */
