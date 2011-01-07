/**
 *  \file TransformationsCluster.h
 *  \brief Class to contain a cluster of Transformation3Ds for a model
              when doing RMSD clustering
 *            according to the valuess of the em2d score
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_TRANSFORMATIONS_CLUSTER_H
#define IMPEM2D_TRANSFORMATIONS_CLUSTER_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/algebra/Transformation3D.h"

IMPEM2D_BEGIN_NAMESPACE

/*!
  Class to guide the building of clusters when doing RMSD clustering
  according to the values of the em2d score.
*/
class IMPEM2DEXPORT TransformationsCluster {
public:

  // Start the cluster
  /*!
    \param[im] t A transformation to begin the cluster
    \param[in] score The score of that transformation. Given that it has to
               be higher to be better, here the ccc is used instead of the em2d
               score

  */
  TransformationsCluster(const algebra::Transformation3D &t,double score):
  representative_trans_(t),representative_em2d_score_(score) {
    all_transformations.push_back(representative_trans_);
  }

  ~TransformationsCluster() {};

  // Function to decide when the representative transformation of
  //   a cluster should be included in this cluster
  /*!
    \param[in] cluster The cluster of tranformations that could be join to
                this one.
  */
  void join_into(const TransformationsCluster &cluster);

  // Returns the score for the representative transformation in this cluster
  double get_score() const {
    return representative_em2d_score_;
  }

  // Updates the total score for the cluster with the one given as parameter
  // It is not necessary to implement it for this type of cluster
  void update_score(float score) {};

  // Returns the representative transformation for this cluster
  algebra::Transformation3D get_representative_transformation() const {
    return representative_trans_;
  }

  algebra::Transformation3D
                get_individual_transformation(unsigned int i) const {
    return all_transformations[i];
  }

  unsigned int get_number_of_transformations() const {
    return all_transformations.size();
  }

  void show(std::ostream &out = std::cout) const {
    out << "Transformations cluster: " << std::endl
        << "Representative transformation: " <<  representative_trans_
        << std::endl << " Number of transfomations in the cluster "
        << get_number_of_transformations() << std::endl;
  }

private:
  // Representative transformation for the cluster
  algebra::Transformation3D representative_trans_;
  // Vector to contain all the transformations in the cluster
  algebra::Transformation3Ds all_transformations;
  // Score of the representative transformation
  double representative_em2d_score_;
};

IMP_VALUES(TransformationsCluster,TransformationsClusters);

IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_TRANSFORMATIONS_CLUSTER_H */
