/**
 *  \file hierarchical_clustering.cpp
 *  \brief Agglomerative clustering algorithm
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/em2d/hierarchical_clustering.h"

IMPEM2D_BEGIN_NAMESPACE

 ClusterSet::ClusterSet(unsigned int N): steps_(0),n_elements_(N) {
    joined_ids1_.resize(n_elements_);
    joined_ids2_.resize(n_elements_);
    clusters_elements.resize(n_elements_);
    // fill unary clusters with clusters id and cluster_distances_
    for (unsigned int i=0;i<n_elements_;++i) {
      joined_ids1_[i]=i;
      joined_ids2_[i]=0; // no clusters joined for the unary ones
      clusters_elements[i].push_back(i);
    }
    cluster_distances_.resize(n_elements_,0.0);
  }

void ClusterSet::do_join_clusters(unsigned int cluster_id1,
                       unsigned int cluster_id2,
                       double distance_between_clusters) {
  joined_ids1_.push_back(cluster_id1);
  joined_ids2_.push_back(cluster_id2);
  cluster_distances_.push_back(distance_between_clusters);
  // join the members of the two clusters
  Ints new_cluster;
  new_cluster.insert(new_cluster.end(),
                     clusters_elements[cluster_id1].begin(),
                     clusters_elements[cluster_id1].end());
  new_cluster.insert(new_cluster.end(),
                     clusters_elements[cluster_id2].begin(),
                     clusters_elements[cluster_id2].end());
  clusters_elements.push_back(new_cluster);
  steps_++;
}


Ints ClusterSet::get_cluster_formed_at_step(unsigned int s) const {
  return get_cluster_elements(s+n_elements_);
}


VectorFloats ClusterSet::get_linkage_matrix() const {
  VectorFloats mat(steps_);
  for (unsigned int i=0;i<steps_;++i) {
    mat[i].resize(3);
    unsigned int j = n_elements_ + i;
    mat[i][0]=(double)joined_ids1_[j];
    mat[i][1]=(double)joined_ids2_[j];
    mat[i][2]= cluster_distances_[j];
  }
  return mat;
}

VectorFloats  ClusterSet::get_linkage_matrix_in_matlab_format() const {
  VectorFloats mat = get_linkage_matrix();
  VectorFloats::iterator it;
  for (it=mat.begin();it != mat.end();++it) {
     (*it)[0] += 1; // +1 for matlab compatibility (indices start at 1)
     (*it)[1] += 1;
  }
  return mat;
}

void  ClusterSet::show(std::ostream &out) const {
  out << " Linkage matrix for the cluster set" << std::endl;
  for (unsigned int i=0;i<joined_ids1_.size();++i) {
    out << joined_ids1_[i] << " "
        << joined_ids2_[i] << " "
        << cluster_distances_[i] << std::endl;
  }
}


double CompleteLinkage::operator()(unsigned int id1,
                unsigned int id2,
                const ClusterSet &cluster_set,
                const VectorFloats &distances ) {
  Ints members1 = cluster_set.get_cluster_elements(id1);
  Ints members2 = cluster_set.get_cluster_elements(id2);
  // Get minimum distance between elements
  Ints::iterator it1,it2;
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


double AverageDistanceLinkage::operator()(unsigned int id1,
                unsigned int id2,
                const ClusterSet &cluster_set,
                const VectorFloats &distances ) {
  Ints members1 = cluster_set.get_cluster_elements(id1);
  Ints members2 = cluster_set.get_cluster_elements(id2);
  // Get minimum distance between elements
  Ints::iterator it1,it2;
  double distance=0.0;
  for (it1=members1.begin();it1 != members1.end();++it1) {
    for (it2=members2.begin();it2 != members2.end();++it2) {
      distance += distances[*it1][*it2];
    }
  }
  return distance/(members1.size()*members2.size());
}

double SingleLinkage::operator()(unsigned int id1,
                unsigned int id2,
                const ClusterSet &cluster_set,
                const VectorFloats &distances ) const {
  Ints members1 = cluster_set.get_cluster_elements(id1);
  Ints members2 = cluster_set.get_cluster_elements(id2);
  // Get minimum distance between elements of the clusters
  Ints::iterator it1,it2;
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


IMPEM2D_END_NAMESPACE
