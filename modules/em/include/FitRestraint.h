/**
 *  \file FitRestraint.h
 *  \brief Calculate score based on fit to EM map.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_FIT_RESTRAINT_H
#define IMPEM_FIT_RESTRAINT_H

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
class IMPEMEXPORT FitRestraint : public Restraint
{
public:
  //! Constructor
  /**
    \param[in] ps The particles participating in the fitting score
    \param[in] em_map  The density map used in the fitting score
    \param[in] refiner rigid body refiner
    \param[in] norm_factors if set, they are used as normalization factors
       for the cross correlation calculations. This is relevant when the
       cross-correlation score of the entire system is decomposed.
    \param[in] weight_key the name of the weight attribute of the particles
    \param[in] scale multiply the fitting restraint score and derivatives
                     by this value
    \note Particles that are rigid-bodies are interpolated and not resampled.
          This significantly reduces the running time but is less accurate.
          If the user prefers to get more accurate results, provide
          its members as input particles and not the rigid body.
    \todo we currently assume rigid bodies are also molecular hierarchies.
   */
  FitRestraint(Particles ps,
               DensityMap *em_map,
               Refiner *refiner,
               FloatPair norm_factors=FloatPair(0.,0.),
               FloatKey weight_key= atom::Mass::get_mass_key(),
               float scale=1,
               bool use_rigid_bodies=true);
  //! \return the predicted density map of the model
  SampledDensityMap * get_model_dens_map() {
    return model_dens_map_;
  }

  IMP_RESTRAINT(FitRestraint);

  IMP_LIST(private, Particle, particle, Particle*, Particles);
private:
  //! Resample the model density map
  void resample() const;
  //! Create density maps: one for each rigid body and one for the rest.
  /**
  \todo the user should pass a refiner for each rigid body. For now we
        assume that each rigid body is a molecular hierarchy
   */
  void initialize_model_density_map(Particles ps,
                                    FloatKey weight_key);

  IMP::internal::OwnerPointer<DensityMap> target_dens_map_;
  mutable IMP::internal::OwnerPointer<SampledDensityMap> model_dens_map_;
  mutable SampledDensityMaps rb_model_dens_map_;
  mutable IMP::internal::OwnerPointer<SampledDensityMap>
    none_rb_model_dens_map_;
  algebra::BoundingBoxD<3> target_bounding_box_;
  // reference to the IMP environment
  float scalefac_;
  core::XYZs xyz_;
  // derivatives
  std::vector<double> dx_, dy_ , dz_;
  //  bool special_treatment_of_particles_outside_of_density_;
  //rigid bodies handling
  Particles all_ps_;
  Particles not_rb_; //all particles that are not part of a rigid body
  core::RigidBodies rbs_;
  Particles mhs_;//mhs_ are the root hierarhices of the rigid bodies
  algebra::ReferenceFrame3Ds rbs_orig_rf_;
  IMP::internal::OwnerPointer<Refiner> refiner_;//refiner for rigid bodies
  FloatKey weight_key_;
  KernelParameters *kernel_params_;
  DistanceMask *dist_mask_;
  FloatPair norm_factors_;
  bool use_rigid_bodies_;
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_FIT_RESTRAINT_H */
