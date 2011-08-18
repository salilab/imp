/**
 *  \file WeightedExcludedVolumeRestraint3.h
 *  \brief Calculate weighted excluded volume between rigid bodies
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_WEIGHTED_EXCLUDED_VOLUME_RESTRAINT_3_H
#define IMPMULTIFIT_WEIGHTED_EXCLUDED_VOLUME_RESTRAINT_3_H

#include "multifit_config.h"
#include <IMP/core/rigid_bodies.h>
#include <IMP/core/DataObject.h>
#include <IMP/algebra/Grid3D.h>

IMPMULTIFIT_BEGIN_NAMESPACE


/** Compute the complementarity between two molecules.
    \note currently each of the particle lists must belong to exactly one
    rigid body. And, currently, each rigid body can only be associated with
    one list of particles (that is, there cannot be multiple restraints with
    different lists of particles that all belong to the same rigid body).
*/
class IMPMULTIFITEXPORT WeightedExcludedVolumeRestraint3 : public Restraint
{
public:
  WeightedExcludedVolumeRestraint3(const ParticlesTemp &a,
                                   const ParticlesTemp &b,
                                   std::string name
                                   ="WeightedExcludedVolumeRestraint3 %1%");
  /** If the two molecules have a penetration score of more than this,
      infinity is returned as the score.*/
  void set_maximum_penetration_score(double s) {
    maximum_penetration_score_=s;
  }
  /** If the two molecules are separated by more than this,
      infinity is returned.
   */
  void set_maximum_separation(double s) {
    maximum_separation_=s;
  }
  /** Set the thickness to use for the external complemenarity layer.*/
  void set_complementarity_thickness(double th) {
    complementarity_thickeness_=th;
    update_voxel();
  }
  /** Set the value to use for external complementarity voxels.
   */
  void set_complementarity_value(double th) {
    complementarity_value_=th;
  }
  /** Set the thickness of the interior layers. */
  void set_interior_layer_thickness(double th) {
    interior_thickness_=th;
    update_voxel();
  }
  IMP_RESTRAINT(WeightedExcludedVolumeRestraint3);
 private:

  void update_voxel();

  ParticlesTemp a_, b_;
  core::RigidBody rba_, rbb_;
  ObjectKey ok_;
  // parameters
  double maximum_separation_, maximum_penetration_score_;
  double complementarity_thickeness_, complementarity_value_;
  double interior_thickness_, voxel_size_;
};

IMPMULTIFIT_END_NAMESPACE

#endif  /* IMPMULTIFIT_WEIGHTED_EXCLUDED_VOLUME_RESTRAINT_3_H */
