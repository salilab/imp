/**
 *  \file FitRestraint.h
 *  \brief Calculate score based on fit to EM map.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
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
    \param[in] radius_key the name of the radius attribute of the particles
    \param[in] weight_key the name of the weight attribute of the particles
    \param[in] scale multiply the fitting restraint score and derivatives
                     by this value
    \param[in] special_treatment_of_particles_outside_of_density
       If more than 80% of the particles are outside of the density
       push it back using upper-bound harmonic
    \param[in] use_fast_version if true densities of rigid-bodies are
               interpolated and not resampled.
    \note In many optimization senarios particles are can be found outside of
  the density. When all particles are outside of the density the
  cross-correlation score is zero and the derivatives are meaningless.
  To handle these cases we guide the particles back into the density by
  using a simple distance restraint between the centroids of the density
  and the particles. Once the particles are back (at least partly) in
  the density, the CC score is back on. To smooth the score,
  we start considering centroids distance once 80% of the particles. This
  option is still experimental and should be used in caution.
    \todo we currently assume rigid bodies are also molecular hierarchies.
   */
  FitRestraint(Particles ps,
               DensityMap *em_map,
               Refiner *refiner,
               FloatKey radius_key= IMP::core::XYZR::get_default_radius_key(),
               FloatKey weight_key= IMP::atom::Mass::get_mass_key(),
               float scale=1,
               bool special_treatment_of_particles_outside_of_density=true,
               bool use_fast_version=true);

  //! \return the predicted density map of the model
  SampledDensityMap *get_model_dens_map() {
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
  void initialize_model_density_map(FloatKey radius_key,
                                    FloatKey weight_key);

  IMP::internal::OwnerPointer<DensityMap> target_dens_map_;
  SampledDensityMap *model_dens_map_;
  std::vector<SampledDensityMap *> rb_model_dens_map_;
  RigidBodiesDerivativesCache rb_rsq_;
  SampledDensityMap * none_rb_model_dens_map_;
  algebra::BoundingBoxD<3> target_bounding_box_;
  // reference to the IMP environment
  float scalefac_;
  IMP::core::XYZs xyz_;
  // derivatives
  std::vector<float> dx_, dy_ , dz_;
  bool special_treatment_of_particles_outside_of_density_;
  bool use_fast_version_;
  //rigid bodies handling
  IMP::Particles not_rb_; //all particles that are not part of a rigid body
  IMP::core::RigidBodies rbs_;
  std::vector<IMP::algebra::Transformation3D> rbs_orig_trans_;
  Refiner *rb_refiner_;//refiner for rigid bodies
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_FIT_RESTRAINT_H */
