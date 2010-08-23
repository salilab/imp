/**
 *  \file WeightedExcludedVolumeRestraint.h
 *  \brief Calculate weighted excluded volume between rigid bodies
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_WEIGHTED_EXCLUDED_VOLUME_RESTRAINT_H
#define IMPMULTIFIT_WEIGHTED_EXCLUDED_VOLUME_RESTRAINT_H

#include "multifit_config.h"
#include <IMP/em/DensityMap.h>
#include <IMP/em/CoarseCC.h>
#include <IMP/em/SurfaceShellDensityMap.h>

#include <IMP/atom/Hierarchy.h>
#include <IMP/core/XYZR.h>
#include <IMP/Model.h>
#include <IMP/Restraint.h>
#include <IMP/VersionInfo.h>
#include <IMP/Refiner.h>

IMPEM_BEGIN_NAMESPACE

//! Calculate score based on fit to EM map
/** \ingroup exp_restraint

 */
class IMPMULTIFITEXPORT WeightedExcludedVolumeRestraint : public Restraint
{
public:
  //! Constructor
  /**
    \param[in] rbs Excluded volume will be calculated between these Rigid bodies
    \param[in] refiner rigid body refiner
    \param[in] radius_key the name of the radius attribute of the particles
    \param[in] weight_key the name of the weight attribute of the particles
    \note Particles that are rigid-bodies are interpolated and not resampled.
          This significantly reduces the running time but is less accurate.
   */
  WeightedExcludedVolumeRestraint(
               core::RigidBodies rbs,
               Refiner *refiner,
               FloatKey radius_key= IMP::core::XYZR::get_default_radius_key(),
               FloatKey weight_key= IMP::atom::Mass::get_mass_key());

  IMP_RESTRAINT(WeightedExcludedVolumeRestraint);

  IMP_LIST(private, Particle, particle, Particle*, Particles);
private:
  //! Create density maps: one for each rigid body
  /**
  \todo the user should pass a refiner for each rigid body. For now we
        assume that each rigid body is a molecular hierarchy
   */
  void initialize_model_density_map(FloatKey radius_key, FloatKey weight_key);
  core::RigidBodies rbs_;
  //one surface map for each rigid body
  mutable SurfaceShellDensityMaps rbs_surface_maps_;
  // reference to the IMP environment
  // derivatives
  std::vector<std::vector<float> >rb_refined_dx_,
                                  rb_refined_dy_ , rb_refined_dz_;
  //  bool special_treatment_of_particles_outside_of_density_;
  //rigid bodies handling
  std::vector<IMP::algebra::Transformation3D> rbs_orig_trans_;
  internal::OwnerPointer<Refiner> rb_refiner_;//refiner for rigid bodies
};

IMPEM_END_NAMESPACE

#endif  /* IMPMULTIFIT_WEIGHTED_EXCLUDED_VOLUME_RESTRAINT_H */
