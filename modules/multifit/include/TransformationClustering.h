/**
 *  \file TransformationClustering.h
 *  \brief Clustering of transformation
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_TRANSFORMATION_CLUSTERING_H
#define IMPMULTIFIT_TRANSFORMATION_CLUSTERING_H

#include "multifit_config.h"
#include <IMP/base_types.h>
#include <IMP/Particle.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/em/rigid_fitting.h>
#include <IMP/domino/DominoSampler.h>
IMPMULTIFIT_BEGIN_NAMESPACE
class IMPMULTIFITEXPORT TransformationClustering {
 public:
  //! Constructor
  /**
     \param[in] ps cluster transformations with respect to this particles
     \param[in] max_rmsd
           if the rmsd between two transformed set of particles
           is smaller then max_angle_diff, then the two transformations
           will be in the same cluster
  */
  TransformationClustering(Particles ps,
                           Float max_rmsd);
  //! fast clustering: rotational parameters + RMSD
  /**
     \param[in] max_angle_diff
         if the angle between two transformed set of particles
         is smaller then max_angle_diff, then the two transformations
         will be in the same cluster
         \param[in] min_cluster_size
         clusters of size smaller than min_rot_cluster_size are ignored
  */
  domino::IntsList cluster_by_rotation(
                const algebra::Transformation3Ds &trans,
                float max_angle_diff_in_rad,int min_cluster_size);

  // fast clustering: rotational&translational parameters
  domino::IntsList cluster_by_transformation(
               const algebra::Transformation3Ds &trans,
               float max_angle_diff,float max_translation_diff,
               int min_cluster_size);
  // RMSD clustering only
  /*
\param[in] trans the transformations to cluster
\param[in] max_rmsd_between_transformations maximum rmsd
between transformations to be considered in the same cluster
\param[in] min_cluster_size clusters of a smaller size will be ignored.
If the value is -1, the member variable min_cluster_size_ will be used
   */
  domino::IntsList cluster_by_rmsd(
                 const algebra::Transformation3Ds &trans,
                 float max_rmsd_between_transformations,
                 int min_cluster_size);

protected:
  // parameters
  Float max_rmsd_;
  Particles ps_;
  //  multifit::RMSDClustering engine_;
};

//! Cluster transformations
/**
\param[in] ps Cluster transformation with respect to these particles
\param[in] max_rmsd Maximum RMSD between transformations within a cluster
\param[in] max_angle_diff_in_rad maximum angle between rotations
in a cluster
\param[in[ max_displace maximum translation between transformations within
a cluster
\param[in] min_cluster_size prune out clusters of size smaller
\return representatives of the clsutered transformations
\note This is one possible protocol for clustering.
For flexibility use internal clustering functions directly.
 */
IMPMULTIFITEXPORT
domino::IntsList get_clustered(Particles ps,
                       const algebra::Transformation3Ds &trans,
                       float max_rmsd,
                       float  max_angle_diff_in_rad,
                       float max_displace,
                       int min_cluster_size);

IMPMULTIFIT_END_NAMESPACE

#endif /* IMPMULTIFIT_TRANSFORMATION_CLUSTERING_H */
