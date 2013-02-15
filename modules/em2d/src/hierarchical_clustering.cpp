/**
 *  \file hierarchical_clustering.cpp
 *  \brief Agglomerative clustering algorithm
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/em2d/hierarchical_clustering.h"

IMPEM2D_BEGIN_NAMESPACE

Ints ClusterSet::get_clusters_below_cutoff(double cutoff) const {
  Ints clusters;
  std::vector<bool> is_active(steps_,true);

  for (int i=steps_-1;i>=0;--i) {
    if(is_active[i]==true && cluster_distances_[i]<cutoff) {
      clusters.push_back(get_id_for_cluster_at_step(i));
      // Deactivate all the members of the linkage matrix that contain
      // the elements of this cluster
      Ints to_deactivate;
      unsigned int  id1 = joined_ids1_[i];
      unsigned int  id2 = joined_ids2_[i];
      if(id1>=n_elements_) to_deactivate.push_back(id1);
      if(id2>=n_elements_) to_deactivate.push_back(id2);

      while(to_deactivate.size() > 0) {
        int id=to_deactivate.back();
        to_deactivate.pop_back();
        int j =get_step_from_id(id); // new row to deactivate
        is_active[j]=false;
        unsigned int  jid1 = joined_ids1_[j];
        unsigned int  jid2 = joined_ids2_[j];
        if(jid1>=n_elements_) to_deactivate.push_back(jid1);
        if(jid2>=n_elements_) to_deactivate.push_back(jid2);
      }
    }
    is_active[i]=false; // deactivate after consdering it
  }
  return clusters;
}



double ClusterSet::get_distance_at_step(unsigned int step) const {
  check_step_value(step);
  return cluster_distances_[step];
}

void ClusterSet::check_step_value(unsigned int s) const {
  if(s>=steps_) IMP_THROW("ClusterSet: Requesting an invalid step",
                                 ValueException);
}


ClusterSet::ClusterSet(unsigned int N): steps_(0),n_elements_(N) {};

void ClusterSet::do_join_clusters(unsigned int cluster_id1,
                       unsigned int cluster_id2,
                       double distance_between_clusters) {
  IMP_LOG_VERBOSE("Joining clusters " << cluster_id1 << " and "
          << cluster_id2 << std::endl);

  joined_ids1_.push_back(cluster_id1);
  joined_ids2_.push_back(cluster_id2);
  cluster_distances_.push_back(distance_between_clusters);


  Ints ids;
  ids.push_back(cluster_id1);
  ids.push_back(cluster_id2);
  Ints new_cluster;
  for (unsigned int i=0;i<2;++i) {
    if( (unsigned int )ids[i]<n_elements_) {
      new_cluster.push_back(ids[i]);
    } else {
      unsigned int s=get_step_from_id(ids[i]);
      new_cluster.insert(new_cluster.end(),
                       clusters_elements_[s].begin(),
                       clusters_elements_[s].end());
    }
  }
  clusters_elements_.push_back(new_cluster);
  steps_++;
}


Ints ClusterSet::get_cluster_formed_at_step(unsigned int s) const {
  check_step_value(s);
  return clusters_elements_[s];
}

Ints ClusterSet::get_cluster_elements(unsigned int id) const {
  if(id<n_elements_) {
    Ints x(1,id);
    return x;
  }
  return get_cluster_formed_at_step(get_step_from_id(id));
}


VectorOfFloats ClusterSet::get_linkage_matrix() const {
  IMP_LOG_VERBOSE("ClusterSet: Building linkage  matrix" << std::endl);
  VectorOfFloats mat(steps_);
  for (unsigned int i=0;i<steps_;++i) {
    mat[i].resize(3);
    mat[i][0] = static_cast<double>(joined_ids1_[i]);
    mat[i][1] = static_cast<double>(joined_ids2_[i]);
    mat[i][2] = cluster_distances_[i];
  }
  return mat;
}

VectorOfFloats  ClusterSet::get_linkage_matrix_in_matlab_format() const {
  VectorOfFloats mat = get_linkage_matrix();
  VectorOfFloats::iterator it;
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
                const VectorOfFloats &distances ) {
  IMP_LOG_VERBOSE("Evaluating CompleteLinkage " << std::endl);
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
                const VectorOfFloats &distances ) {
  IMP_LOG_VERBOSE("Evaluating AverageDistanceLinkage " << std::endl);

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
                const VectorOfFloats &distances ) const {
  IMP_LOG_VERBOSE("Evaluating SingleLinkage " << std::endl);
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
