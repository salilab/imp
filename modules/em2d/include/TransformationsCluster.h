/**
 *  \file TransformationsCluster.h
 *  \brief Class to contain a cluster of Transformation3Ds for a model
              when doing RMSD clustering
 *            according to the valuess of the em2d score
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_TRANSFORMATIONS_CLUSTER_H
#define IMPEM2D_TRANSFORMATIONS_CLUSTER_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/algebra/Transformation3D.h"

IMPEM2D_BEGIN_NAMESPACE

/*!
  Class to contain clusters of transformations.
  \note The clustering function works with higher scores for better models
*/
class IMPEM2DEXPORT TransformationsCluster {
public:

  // Start the cluster
  /*!
    \param[in] t A transformation to begin the cluster
    \param[in] score The score of that transformation. the higher, the better
    \param[in] model_id identifies the model the initial transformation
                belongs to. Set to  0 if you don't need that information.
  */
  TransformationsCluster(const algebra::Transformation3D &t,
                         double score,unsigned int model_id):
              representative_trans_(t),  representative_score_(score),
              representative_model_id_(model_id) {
    all_transformations_.push_back(representative_trans_);
    all_models_ids_.push_back(representative_model_id_);
    all_scores_.push_back(representative_score_);
  }

  ~TransformationsCluster() {};

  // Merge clusters. Copies all transformations from the other cluster and
  // updates the representative one if necessary
  void join_into(const TransformationsCluster &oher);

  // Returns the score for the representative transformation in this cluster
  double get_score() const {
    return representative_score_;
  }

  // Updates the total score for the cluster with the one given as parameter
  // It is not necessary to implement it for this type of cluster
  void update_score(float score) {};

  // Returns the representative transformation for this cluster
  algebra::Transformation3D get_representative_transformation() const {
    return representative_trans_;
  }

  unsigned int get_representative_model_id() const {
    return representative_model_id_;
  }


  // Get the transformation i of the cluster
  algebra::Transformation3D
                get_individual_transformation(unsigned int i) const {
    return all_transformations_[i];
  }

  // Id of the model stored in the transformation i
  unsigned int get_individual_model_id(unsigned int i  ) const {
    return all_models_ids_[i];
  }

  double get_individual_score(unsigned int i) const {
    return all_scores_[i];
  }

  unsigned int get_number_of_members() const {
    return all_transformations_.size();
  }


  void show(std::ostream &out = std::cout) const {
    out << "Transformations cluster: " << get_number_of_members()
        << " elements." << std::endl
        << "Representative transformation: " <<  representative_trans_
        << std::endl;
  }

private:
  algebra::Transformation3D representative_trans_;
  double representative_score_;
  unsigned int representative_model_id_;
  algebra::Transformation3Ds all_transformations_;
  Ints all_models_ids_;
  Floats all_scores_;
};

IMP_VALUES(TransformationsCluster,TransformationsClusters);

IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_TRANSFORMATIONS_CLUSTER_H */
