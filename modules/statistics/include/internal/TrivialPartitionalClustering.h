/**
 *  \file random_generator.h   \brief random number generator
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_INTERNAL_TRIVIAL_PARTITIONAL_CLUSTERING_H
#define IMPSTATISTICS_INTERNAL_TRIVIAL_PARTITIONAL_CLUSTERING_H

#include "IMP/random.h"
#include <IMP/statistics/statistics_config.h>
#include "../PartitionalClustering.h"
#include "../partitional_clustering_macros.h"
#include <IMP/base/check_macros.h>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adjacency_list.hpp>
IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE
class TrivialPartitionalClustering : public PartitionalClustering {
  IMP::base::Vector<Ints> clusters_;
  Ints reps_;

 public:
  TrivialPartitionalClustering(const IMP::base::Vector<Ints> &clusters,
                               const Ints &reps = Ints())
      : PartitionalClustering("trivial"), clusters_(clusters), reps_(reps) {
    if (reps.empty()) {
      reps_.resize(clusters_.size());
      for (unsigned int i = 0; i < reps_.size(); ++i) {
        reps_[i] = clusters[i][0];
      }
    }
  }
  IMP_PARTITIONAL_CLUSTERING(TrivialPartitionalClustering);
};

inline unsigned int
TrivialPartitionalClustering::get_number_of_clusters() const {
  IMP_CHECK_OBJECT(this);
  return clusters_.size();
}
inline const Ints &TrivialPartitionalClustering::get_cluster(
    unsigned int i) const {
  IMP_CHECK_OBJECT(this);
  IMP_USAGE_CHECK(
      i < get_number_of_clusters(),
      "There are only " << get_number_of_clusters() << " clusters. Not " << i);
  set_was_used(true);
  return clusters_[i];
}
inline int TrivialPartitionalClustering::get_cluster_representative(
    unsigned int i) const {
  IMP_CHECK_OBJECT(this);
  IMP_USAGE_CHECK(
      i < get_number_of_clusters(),
      "There are only " << get_number_of_clusters() << " clusters. Not " << i);
  return reps_[i];
}
inline void TrivialPartitionalClustering::do_show(std::ostream &out) const {
  out << clusters_.size() << " centers." << std::endl;
}

IMPSTATISTICS_END_INTERNAL_NAMESPACE

#endif /* IMPSTATISTICS_INTERNAL_TRIVIAL_PARTITIONAL_CLUSTERING_H */
