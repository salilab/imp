/**
 *  \file RigidBodiesFitRestraint.h
 *  \brief Calculate score based on fit to EM map for a set of rigid bodies
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_RIGID_BODIES_FIT_RESTRAINT_H
#define IMPEM_RIGID_BODIES_FIT_RESTRAINT_H

#include "em_config.h"
#include "DensityMap.h"
#include "CoarseCC.h"
#include "SampledDensityMap.h"

#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Mass.h>
#include <IMP/core/XYZR.h>
#include <IMP/Model.h>
#include <IMP/Restraint.h>
#include <IMP/VersionInfo.h>
#include <IMP/Refiner.h>

IMPEM_BEGIN_NAMESPACE

//! Calculate score based on fit to EM map
/** \ingroup exp_restraint

 */
class IMPEMEXPORT RigidBodiesFitRestraint : public Restraint
{
public:
  //! Constructor
  /**
    \param[in] rbs The rigid bodies participating in the fitting score
    \param[in] em_map  The density map used in the fitting score
    \param[in] refiner rigid body refiner
    \param[in] radius_key the name of the radius attribute of the particles
    \param[in] weight_key the name of the weight attribute of the particles
    \param[in] scale multiply the fitting restraint score and derivatives
                     by this value
   */
  RigidBodiesFitRestraint(core::RigidBodies rbs,
               DensityMap *em_map,
               Refiner *refiner,
               FloatKey radius_key= IMP::core::XYZR::get_default_radius_key(),
               FloatKey weight_key= IMP::atom::Mass::get_mass_key(),
               float scale=1);
  //! \return the predicted density map of the model
  SampledDensityMap * get_model_dens_map() {
    return model_dens_map_;
  }

  IMP_RESTRAINT(RigidBodiesFitRestraint);

  IMP_LIST(private, Particle, particle, Particle*, Particles);
private:
  //! Resample the model density map
  void resample() const;
  //! Sample the contribution of a rigid body particles to each voxel
  void sample_rigid_body_derivatives_map(
    core::RigidBody rb, algebra::Vector3Ds &d_vecs) const;
  //fast calculation of derivatives for a rigid body
  algebra::Vector3D calculate_rigid_body_derivatives(int rb_ind) const;
  //! Create density maps: one for each rigid body and one for the rest.
  /**
  \todo the user should pass a refiner for each rigid body. For now we
        assume that each rigid body is a molecular hierarchy
   */
  void initialize_model_density_map(FloatKey radius_key,
                                    FloatKey weight_key);

  IMP::internal::OwnerPointer<DensityMap> target_dens_map_;
  mutable internal::OwnerPointer<SampledDensityMap> model_dens_map_;
  mutable SampledDensityMaps rb_model_dens_map_;
  //for each density map we hold a vector of Vector3Ds, each entry stores
  //the sum of direvatives directions for all particles of the rigid body
  mutable std::vector<algebra::Vector3Ds> rb_derivatives_dens_map_;
  algebra::BoundingBoxD<3> target_bounding_box_;
  // reference to the IMP environment
  float scalefac_;
  IMP::core::XYZs xyz_;
  // derivatives
  std::vector<float> dx_, dy_ , dz_;
  bool special_treatment_of_particles_outside_of_density_;
  IMP::core::RigidBodies rbs_;
  std::vector<IMP::algebra::Transformation3D> rbs_orig_trans_;
  internal::OwnerPointer<Refiner> rb_refiner_;//refiner for rigid bodies
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_RIGID_BODIES_FIT_RESTRAINT_H */
