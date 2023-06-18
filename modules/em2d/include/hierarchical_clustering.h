/**
 *  \file IMP/em2d/hierarchical_clustering.h
 *  \brief Agglomerative clustering algorithm
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_HIERARCHICAL_CLUSTERING_H
#define IMPEM2D_HIERARCHICAL_CLUSTERING_H

#include <IMP/em2d/em2d_config.h>
#include "IMP/em2d/scores2D.h"
#include <IMP/em2d/internal/clustering_helper.h>
#include "IMP/base_types.h"
#include <IMP/log.h>
#include <vector>
#include <list>
#include <algorithm>
#include <limits>
#include <functional>
#include <cereal/access.hpp>

IMPEM2D_BEGIN_NAMESPACE

template <class T>
void print_vector(const std::vector<T> &v) {
  for (unsigned int i = 0; i < v.size(); ++i) {
    std::cout << v[i] << " , ";
  }
  std::cout << std::endl;
}

//! A class to store the clusters generated during hierarchical clustering
class IMPEM2DEXPORT ClusterSet {
 public:
  ClusterSet() : steps_(0), n_elements_(0) {}

  /**
    \param[in] N Number of elements to be clustered
  */
  ClusterSet(unsigned int N);

  //! join operation
  /** \param[in] cluster_id1 id of the 1st cluster joined
      \param[in] cluster_id2 id of the 2nd cluster merged
      \param[in] distance_between_clusters distance between the merged clusters
  */
  void do_join_clusters(unsigned int cluster_id1, unsigned int cluster_id2,
                        double distance_between_clusters);

  //! Returns a vector with the ids of the elements that are in a cluster
  /** Does not contain any hierarchical information, just the members
      \param[in] id of the cluster
  */
  Ints get_cluster_elements(unsigned int id) const;

  //! Get the biggest clusters that have distances below a given cutoff
  /** \param[in] cutoff distance
      \return A vector of Ints: Each Ints has the ids of all elements of the
              cluster
  */
  Ints get_clusters_below_cutoff(double cutoff) const;

  //! Return the elements of the cluster formed at a given step
  Ints get_cluster_formed_at_step(unsigned int step) const;

  //! Distance in the linkage matrix at a given step
  double get_distance_at_step(unsigned int step) const;

  unsigned int get_id_for_cluster_at_step(unsigned int step) const {
    return step + n_elements_;
  }

  //! Returns the linkage matrix
  /**
    \note Linkage matrix is a matrix A[N-1][3].
    A[i][0] - id of the first cluster merged at step i
    A[i][1] - id of the second cluster merged at step i
    A[i][2] - distance between the clusters
  */
  FloatsList get_linkage_matrix() const;

  //! Returns the linkage matrix compatible with Matlab format
  /**
    \note This function merely adds 1 to the cluster ids, for compatibility
     with Matlab.
    Matlab format: http://www.mathworks.com/help/toolbox/stats/linkage.html
  */
  FloatsList get_linkage_matrix_in_matlab_format() const;

  //! Returns the number of steps of clustering recorded
  unsigned int get_number_of_steps() const { return steps_; }

  void show(std::ostream &out) const;

 private:
  void check_step_value(unsigned int s) const;
  unsigned int get_step_from_id(unsigned int id) const {
    return (id - n_elements_);
  }

  unsigned int steps_;
  unsigned int n_elements_;  // number of elements to cluster
  Ints joined_ids1_, joined_ids2_;
  Floats cluster_distances_;
  // each element of the outermost vector is a vector with all the elements
  // in a cluster
  IntsList clusters_elements_;

private:
  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(steps_, n_elements_, joined_ids1_, joined_ids2_,
       cluster_distances_, clusters_elements_);
  }
};
IMP_VALUES(ClusterSet, ClusterSets);

//! Functor for hierarchical clustering based on single linkage
class IMPEM2DEXPORT SingleLinkage {
 public:
  SingleLinkage() {}
  /**
    \param[in] id1 identity of cluster 1 to merge
    \param[in] id2 identity of cluster 2 to merge
    \param[in] cluster_set linkage matrix describing the contents of
               clusters so far.
    \param[in] distances A NxN matrix of distances(i,j) between the individual
              members to cluster
    \return Minimal distance between members of the clusters
    \note the id of an isolated member n<N is n. The id of the cluster formed
          at step i is i+N.
  */
  double operator()(unsigned int id1, unsigned int id2,
                    const ClusterSet &cluster_set,
                    const FloatsList &distances) const;

  void show(std::ostream &out) const {
    out << "SingleLinkage";
  };
private:
  friend class cereal::access;

  template<class Archive> void serialize(Archive &) {}
};
IMP_VALUES(SingleLinkage, SingleLinkages);

//! Functor for hierarchical clustering based on complete linkage
class IMPEM2DEXPORT CompleteLinkage {
 public:
  CompleteLinkage() {}
  //! Distance between the clusters
  /**
    \note See SingleLinkage class for the meaning of the arguments
    \return Maximal distance between 2 members in the merged cluster

  */
  double operator()(unsigned int id1, unsigned int id2,
                    const ClusterSet &cluster_set, const FloatsList &distances);

  void show(std::ostream &out) const {
    out << "CompleteLinkage";
  };
private:
  friend class cereal::access;

  template<class Archive> void serialize(Archive &) {}
};
IMP_VALUES(CompleteLinkage, CompleteLinkages);

//! Functor for hierarchical clustering based on average-linkage
class IMPEM2DEXPORT AverageDistanceLinkage {
 public:
  AverageDistanceLinkage() {}
  //! Distance between the clusters
  /**
    \note See SingleLinkage class for the meaning of the arguments
    \return Average between all members of the merged cluster
  */
  double operator()(unsigned int id1, unsigned int id2,
                    const ClusterSet &cluster_set, const FloatsList &distances);

  void show(std::ostream &out) const {
    out << "AverageDistanceLinkage";
  };
private:
  friend class cereal::access;

  template<class Archive> void serialize(Archive &) {}
};
IMP_VALUES(AverageDistanceLinkage, AverageDistanceLinkages);

//! Function to perform agglomerative clustering
/**
  \param[in] distances Vector of Floats containing all the
  possible distances(i,j) between elements to cluster. Given N elements to
  cluster, there are N vectors of size N
  \return a ClusterSet class containing all the clustering steps.
*/
template <class LinkageFunction>
ClusterSet do_hierarchical_agglomerative_clustering(
    const FloatsList &distances) {
  // Based on:
  // http://nlp.stanford.edu/IR-book/html/htmledition/
  //      time-complexity-of-hac-1.html)

  IMP_LOG_TERSE("starting hierarchical_agglomerative_clustering " << std::endl);

  unsigned int N = distances.size();  // number of elements
                                      // Lists of distances between elements
  // List n has members (i,distance_n_i).
  std::vector<internal::list_cluster_id_distance> lists(N);
  // id of the cluster associated with each list
  Ints cluster_id(N);
  // All list active at the beginning
  std::vector<bool> active_list(N);
  std::fill(active_list.begin(), active_list.end(), true);
  // Fill lists
  for (unsigned int n = 0; n < N; ++n) {
    for (unsigned int i = 0; i < N; ++i) {
      if (i != n) {
        lists[n].push_back(std::make_pair(i, distances[n][i]));
      }
    }
    lists[n].sort(LessPairBySecond<internal::pair_cluster_id_distance>());
    // At the beginning each list is associated with a cluster of one element
    cluster_id[n] = n;
  }

  ClusterSet cluster_set(N);
  LinkageFunction linkage_function;
  unsigned int steps = N - 1;  // Steps of clustering
  // cluster algorithm
  for (unsigned int k = 0; k < steps; ++k) {
    IMP_LOG_TERSE(std::endl);
    // Find the list that contains lower distance
    double minimum_distance = std::numeric_limits<double>::max();
    unsigned int l1 = 0;

    for (unsigned int j = 0; j < N; ++j) {
      if (active_list[j] == true) {
        // closest distance for list j
        if (lists[j].front().second < minimum_distance) {
          minimum_distance = lists[j].front().second;
          l1 = j;
        }
      }
    }  // list l1 contains lowest distance
    // lowest distance is between list l1 and list l2
    unsigned int l2 = lists[l1].front().first;
    minimum_distance = lists[l2].front().second;
    IMP_LOG_TERSE("step " << k << ": joining clusters " << cluster_id[l1]
                          << " and " << cluster_id[l2] << " to form cluster "
                          << cluster_set.get_id_for_cluster_at_step(k)
                          << " distance = " << minimum_distance << std::endl);
    cluster_set.do_join_clusters(cluster_id[l1], cluster_id[l2],
                                 minimum_distance);
    active_list[l2] = false;
    cluster_id[l1] = cluster_set.get_id_for_cluster_at_step(k);
    // clear list 1. It will be filled with distance values for the new cluster
    lists[l1].clear();
    // Update lists of distances
    for (unsigned int i = 0; i < N; ++i) {
      IMP_LOG_TERSE("Updating list of distances " << i << std::endl);
      if (active_list[i] == true && i != l1) {
        IMP_LOG_TERSE("List " << i << " is active " << std::endl);
        // Delete list elements that store distances to the merged clusters
        lists[i].erase(std::remove_if(lists[i].begin(), lists[i].end(),
                                      internal::ListHasDistance(l1, l2)),
                       lists[i].end());
        // Update distances to the merged cluster
        double dist = linkage_function(cluster_id[l1], cluster_id[i],
                                       cluster_set, distances);
        IMP_LOG_TERSE("Distance by linkage function " << dist << std::endl);
        lists[i].push_back(std::make_pair(l1, dist));
        lists[l1].push_back(std::make_pair(i, dist));
      }
    }
    // Sort lists
    for (unsigned int i = 0; i < N; ++i) {
      if (active_list[i] == true) {
        lists[i].sort(LessPairBySecond<internal::pair_cluster_id_distance>());
      }
    }
  }
  return cluster_set;
}

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_HIERARCHICAL_CLUSTERING_H */
