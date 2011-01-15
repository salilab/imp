/**
 *  \file hierarchical_clustering.h
 *  \brief Agglomerative clustering algorithm
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_HIERARCHICAL_CLUSTERING_H
#define IMPEM2D_HIERARCHICAL_CLUSTERING_H

#include "IMP/em2d/em2d_config.h"
#include <vector>
#include <list>
#include <algorithm>
#include <limits>

IMPEM2D_BEGIN_NAMESPACE

typedef std::pair<unsigned int ,double> pair_index_distance;
typedef std::list< pair_index_distance > list_index_distance;
typedef std::vector< Floats > VectorFloats;


// Comparison of cluster_id,distance
class LessThanByDistance {
public:
  bool operator()(const pair_index_distance &a,
                  const pair_index_distance &b) {
    return a.second < b.second;
  }
  void show(std::ostream &out) const {};
};

IMP_VALUES(LessThanByDistance,LessThanByDistances);

// A class to store the clusters generated during hierarchical clustering;
class IMPEM2DEXPORT ClusterSet {
public:

  /*!
    \param[in] N Number of elements to be clustered
  */
  ClusterSet(unsigned int N): steps_(0),N_(N) {
    joined_ids1_.resize(N_);
    joined_ids2_.resize(N_);
    clusters_elements.resize(N_);
    // fill unary clusters with clusters id and cluster_distances_
    for (unsigned int i=0;i<N_;++i) {
      joined_ids1_[i]=i;
      joined_ids2_[i]=0; // no clusters joined for the unary ones
      clusters_elements[i].push_back(i);
    }
    cluster_distances_.resize(N_,0.0);
  }

  // join operation
  /*!
    \param[in] id of the 1st cluster joined
    \param[in] id of the 2nd cluster merged
    \param[in] distance between the merged clusters
  */
  void do_join_clusters(unsigned int cluster_id1,
                         unsigned int cluster_id2,
                         double distance_between_clusters) {
    joined_ids1_.push_back(cluster_id1);
    joined_ids2_.push_back(cluster_id2);
    cluster_distances_.push_back(distance_between_clusters);
    // join the members of the two clusters
    std::vector<unsigned int> new_cluster;
    new_cluster.insert(new_cluster.end(),
                       clusters_elements[cluster_id1].begin(),
                       clusters_elements[cluster_id1].end());
    new_cluster.insert(new_cluster.end(),
                       clusters_elements[cluster_id2].begin(),
                       clusters_elements[cluster_id2].end());
    clusters_elements.push_back(new_cluster);
    steps_++;
  }

   // Returns a vector with the ids of the elements that are in a cluster
   // Does not contain any hierarchical information, just the members
   /*!
     \param[in] id of the cluster
   */
   std::vector<unsigned int > get_cluster_elements(unsigned int id) const {
     return clusters_elements[id];
   }

  // Distance between two clusters
  double get_distance(unsigned int cluster_id1,
                      unsigned int cluster_id2) const {
    return 0.0;//TO DO
  }

  // Returns a linkage matrix compatible with Matlab format
  /*!
    \note Linkage matrix is a matrix A[N-1][3].
    Matlab format: http://www.mathworks.com/help/toolbox/stats/linkage.html
    A[i][0] - id of the first cluster merged at step i
    A[i][1] - id of the second cluster merged at step i
    A[i][2] - distance between the clusters
  */
  VectorFloats get_linkage_matrix() const {
    VectorFloats mat(steps_);
    for (unsigned int i=0;i<steps_;++i) {
      mat[i].resize(3);
      mat[i][0]=(double)joined_ids1_[i];
      mat[i][1]=(double)joined_ids2_[i];
      mat[i][2]= cluster_distances_[i];
    }
    return mat;
  }

  void show(std::ostream &out) const {
    out << " Linkage matrix for the cluster set" << std::endl;
    for (unsigned int i=0;i<steps_;++i) {
      out << joined_ids1_[i] << " "
          << joined_ids2_[i] << " "
          << cluster_distances_[i] << std::endl;
    }
  }

private:
  unsigned int steps_;
  unsigned int N_; // number of elements
  std::vector<unsigned int> joined_ids1_,joined_ids2_;
  Floats cluster_distances_;
  // each element of the outermost vector is a vector with all the elements
  // in a cluster
  std::vector< std::vector<unsigned int> > clusters_elements;
};

IMP_VALUES(ClusterSet,ClusterSets);

// Functor for hierarchical clustering based on single linkage
class IMPEM2DEXPORT SingleLinkage {
public:
  /*!
    \param[in] id1 identity of cluster 1 to merge
    \param[in] id2 identity of cluster 2 to merge
    \param[in] linkage matrix describing the contents of clusters so far.
    \param[in] distances. A NxN matrix of distances(i,j) between the individual
              members to cluster
    \param[out] Minimal distance between members of the clusters
    \note the id of an isolated member n<N is n. The id of the cluster formed
          at step i is i+N.
  */
  double operator()(unsigned int id1,
                  unsigned int id2,
                  const ClusterSet &cluster_set,
                  const VectorFloats &distances ) const {
    std::vector<unsigned int> members1 = cluster_set.get_cluster_elements(id1);
    std::vector<unsigned int> members2 = cluster_set.get_cluster_elements(id2);
    // Get minimum distance between elements
    std::vector<unsigned int>::iterator it1,it2;
    double minimum_distance=std::numeric_limits<double>::max();
    double distance=0.0;
    for (it1=members1.begin();it1 != members1.end();++it1) {
      for (it2=members2.begin();it2 != members2.end();++it2) {
        distance = distances[*it1][*it2];
        if(distance < minimum_distance) minimum_distance = distance;
      }
    }
    return minimum_distance;
  }

  void show(std::ostream &out) const {};
};

IMP_VALUES(SingleLinkage,SingleLinkages);

// Functor for hierarchical clustering based on complete linkage
class IMPEM2DEXPORT CompleteLinkage {
public:
  // Distance between the clusters
  /*!
    \note See SingleLinkage class for the meaning of the arguments
    \param[out] Maximal distance between 2 members in the merged cluster

  */
  double operator()(unsigned int id1,
                  unsigned int id2,
                  const ClusterSet &cluster_set,
                  const VectorFloats &distances ) {
    std::vector<unsigned int> members1 = cluster_set.get_cluster_elements(id1);
    std::vector<unsigned int> members2 = cluster_set.get_cluster_elements(id2);
    // Get minimum distance between elements
    std::vector<unsigned int>::iterator it1,it2;
    double maximum_distance=std::numeric_limits<double>::min();
    double distance=0.0;
    for (it1=members1.begin();it1 != members1.end();++it1) {
      for (it2=members2.begin();it2 != members2.end();++it2) {
        distance = distances[*it1][*it2];
        if(distance > maximum_distance) maximum_distance = distance;
      }
    }
    return maximum_distance;
  }

  void show(std::ostream &out) const {};

};

IMP_VALUES(CompleteLinkage,CompleteLinkages);

// Functor for hierarchical clustering based on average-linkage
class IMPEM2DEXPORT AverageDistance {
public:
  // Distance between the clusters
  /*!
    \note See SingleLinkage class for the meaning of the arguments
    \param[out] Average between all members of the merged cluster
  */
  double operator()(unsigned int id1,
                  unsigned int id2,
                  const ClusterSet &cluster_set,
                  const VectorFloats &distances ) {
    std::vector<unsigned int> members1 = cluster_set.get_cluster_elements(id1);
    std::vector<unsigned int> members2 = cluster_set.get_cluster_elements(id2);
    // Get minimum distance between elements
    std::vector<unsigned int>::iterator it1,it2;
    double distance=0.0;
    for (it1=members1.begin();it1 != members1.end();++it1) {
      for (it2=members2.begin();it2 != members2.end();++it2) {
        distance += distances[*it1][*it2];
      }
    }
    return distance/(members1.size()*members2.size());
  }

  void show(std::ostream &out) const {};
};

IMP_VALUES(AverageDistance,AverageDistances);

// Function to perform agglomerative clustering
/*!
  \param[in] distances Vector of Floats containing all the
  possible distances(i,j) between elements to cluster. Given N elements to
  cluster, there are N vectors of size N
  \param[in] Function to calculate distance between clusters.
  \param[out] a ClusterSet class containing all the clustering steps.
*/
template<class LinkageFunction>
ClusterSet
    hierarchical_agglomerative_clustering(const VectorFloats &distances) {
// Based on:
// http://nlp.stanford.edu/IR-book/html/htmledition/
//      time-complexity-of-hac-1.html)

  IMP_LOG(IMP::TERSE,
            "starting hierarchical_agglomerative_clustering " << std::endl);

  unsigned int N = distances.size(); // number of elements
  // Lists of distances between elements
  // List n has members (i,distance_n_i).
  std::vector< list_index_distance > lists(N);
  // id of the cluster associated with each list
  std::vector<unsigned int> cluster_id(N);
  // All list active at the beginning
  std::vector<bool> active_list(N);
  std::fill(active_list.begin(),active_list.end(),true);
  // Fill lists
  for (unsigned int n=0;n<N;++n) {
    for(unsigned int i=0;i<N;++i) {
      if(i != n) {
        lists[n].push_back(std::make_pair(i,distances[n][i]));
      }
    }
    lists[n].sort(LessThanByDistance());
    // At the beginning each list is associated with a cluster of one element
    cluster_id[n]=n;
  }
  IMP_LOG(IMP::TERSE,"lists are built " << std::endl);


  ClusterSet cluster_set(N);
  LinkageFunction linkage_function;
  unsigned int steps = N-1;// Steps of clustering
  // cluster algorithm
  for (unsigned int k=0;k<steps;++k) {
    IMP_LOG(IMP::TERSE,"step " << k << std::endl);
    // Find the list that contains lower distance
    double minimum_distance=std::numeric_limits<double>::max();
    unsigned int l1=0;
    for (unsigned int j=0;j<N;++j) {
      if(active_list[j]==true) {
        // closest distance for list j
        if(lists[j].front().second < minimum_distance) {
          minimum_distance=lists[j].front().second;
          l1=j;
        }
      }
    } // list l1 contains lowest distance
    // lowest distance is between list l1 and list l2
    unsigned int l2=lists[l1].front().first;
    minimum_distance=lists[l2].front().second;
    IMP_LOG(IMP::VERBOSE,"joining clusters " << cluster_id[l1]
            << " and " << cluster_id[l2] << std::endl);

    // store the linkage of clusters
    cluster_set.do_join_clusters(cluster_id[l1],
                                 cluster_id[l2],
                                 minimum_distance);
    active_list[l2]=false;
    // the new cluster after joining has id = step+n_elements.
    cluster_id[l1]=k+N;
    // clear list 1. It will be filled with distance values for the new cluster
    lists[l1].clear();
    // Update lists of distances
    for (unsigned int i=0;i<N;++i) {
      IMP_LOG(IMP::VERBOSE,"Updating list of distances " << i << std::endl);
      if(active_list[i]==true && i!=l1) {
        // Delete list elements that store distances to the merged clusters
        list_index_distance::iterator it;
        for (it=lists[i].begin() ; it!=lists[i].end() ; ++it) {
          if((*it).first == l1 || (*it).first == l2 ) lists[i].erase(it);
        }
        // Update distances to the merged cluster
        double dist = linkage_function(cluster_id[l1],
                                      cluster_id[i],
                                      cluster_set,
                                      distances);
      IMP_LOG(IMP::VERBOSE,"After linkage function" << std::endl);
        lists[i].push_back(std::make_pair(l1,dist));
        lists[l1].push_back(std::make_pair(i,dist));
      }
    }
    // Sort lists
    for (unsigned int i=0;i<N;++i) {
      if(active_list[i]==true) lists[i].sort(LessThanByDistance());
    }
  }
  return cluster_set;
}

IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_HIERARCHICAL_CLUSTERING_H */
