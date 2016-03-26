/**
 *  \file random_generator.h   \brief random number generator
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_INTERNAL_TRIVIAL_PARTITIONAL_CLUSTERING_H
#define IMPSTATISTICS_INTERNAL_TRIVIAL_PARTITIONAL_CLUSTERING_H

#include <IMP/statistics/statistics_config.h>
#include "../PartitionalClustering.h"
#include <IMP/check_macros.h>

// Work around Boost bug with adjacency_matrix in 1.60:
// https://svn.boost.org/trac/boost/ticket/11880
#include <boost/version.hpp>
#if BOOST_VERSION == 106000
# include <boost/type_traits/ice.hpp>
#endif

#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adjacency_list.hpp>

IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE
class TrivialPartitionalClustering : public PartitionalClustering {
  IMP::Vector<Ints> clusters_;
  Ints reps_;

 public:
  TrivialPartitionalClustering(const IMP::Vector<Ints> &clusters,
                               const Ints &reps = Ints())
      : PartitionalClustering("trivial"), clusters_(clusters), reps_(reps) {
    if (reps.empty()) {
      reps_.resize(clusters_.size());
      for (unsigned int i = 0; i < reps_.size(); ++i) {
        reps_[i] = clusters[i][0];
      }
    }
  }
  unsigned int get_number_of_clusters() const IMP_OVERRIDE;
  const Ints &get_cluster(unsigned int i) const IMP_OVERRIDE;
  int get_cluster_representative(unsigned int i) const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(TrivialPartitionalClustering);
};

inline unsigned int TrivialPartitionalClustering::get_number_of_clusters()
    const {
  IMP_CHECK_OBJECT(this);
  return clusters_.size();
}
inline const Ints &TrivialPartitionalClustering::get_cluster(unsigned int i)
    const {
  IMP_CHECK_OBJECT(this);
  IMP_USAGE_CHECK(i < get_number_of_clusters(), "There are only "
                                                    << get_number_of_clusters()
                                                    << " clusters. Not " << i);
  set_was_used(true);
  return clusters_[i];
}
inline int TrivialPartitionalClustering::get_cluster_representative(
    unsigned int i) const {
  IMP_CHECK_OBJECT(this);
  IMP_USAGE_CHECK(i < get_number_of_clusters(), "There are only "
                                                    << get_number_of_clusters()
                                                    << " clusters. Not " << i);
  return reps_[i];
}

IMPSTATISTICS_END_INTERNAL_NAMESPACE

#endif /* IMPSTATISTICS_INTERNAL_TRIVIAL_PARTITIONAL_CLUSTERING_H */
