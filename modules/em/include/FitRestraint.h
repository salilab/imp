/**
 *  \file IMP/em/FitRestraint.h
 *  \brief Calculate score based on fit to EM map.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_FIT_RESTRAINT_H
#define IMPEM_FIT_RESTRAINT_H

#include <IMP/em/em_config.h>
#include "DensityMap.h"
#include "CoarseCC.h"
#include "SampledDensityMap.h"

#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Mass.h>
#include <IMP/core/XYZR.h>
#include <IMP/Model.h>
#include <IMP/Restraint.h>
#include <IMP/Refiner.h>
#include <boost/unordered_map.hpp>

IMPEM_BEGIN_NAMESPACE

//! Calculate score based on fit to EM map
/** The score is a function of the cross correlation between the
    EM map and the map simulated from the particles. This is calculated
    by means of a double sum over the two sets of voxels.

    \note In most cases it is faster to approximate the two maps with
          a Gaussian Mixture Model and to use that for the scoring, with
          isd::GaussianEMRestraint.

    \ingroup exp_restraint
 */
class IMPEMEXPORT FitRestraint : public Restraint {
 public:
  //! Constructor
  /**
    \param[in] ps The particles participating in the fitting score
    \param[in] em_map  The density map used in the fitting score
    \param[in] norm_factors if set, they are used as normalization factors
       for the cross correlation calculations. This is relevant when the
       cross-correlation score of the entire system is decomposed.
    \param[in] weight_key the name of the weight attribute of the particles
    \param[in] scale multiply the fitting restraint score and derivatives
                     by this value
    \param[in] use_rigid_bodies if some of the particles are part of
               a rigid body,
               use the rigid body for faster calculations
    \param[in] kt KernelType to use for simulating density from particles
    \note Rigid-bodies are interpolated and not resampled
          if use_rigid_bodies option is selected.
          This significantly reduces the running time but is less accurate.
          If the user prefers to get more accurate results, provide
          its members as input particles and not the rigid body.
   */
  FitRestraint(ParticlesTemp ps, DensityMap *em_map,
               FloatPair norm_factors = FloatPair(0., 0.),
               FloatKey weight_key = atom::Mass::get_mass_key(),
               float scale = 1, bool use_rigid_bodies = true,
               KernelType kt = GAUSSIAN);
  //! Return the predicted density map of the model
  SampledDensityMap *get_model_dens_map() const { return model_dens_map_; }
  void set_scale_factor(float scale) { scalefac_ = scale; }
  float get_scale_factor() const { return scalefac_; }
  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(FitRestraint);

#ifndef SWIG
  IMP_LIST(private, Particle, particle, Particle *, Particles);
#endif
 private:
  //! Store particles
  void store_particles(ParticlesTemp ps);
  //! Resample the model density map
  void resample() const;
  //! Create density maps: one for each rigid body and one for the rest.
  void initialize_model_density_map(FloatKey weight_key);

  IMP::PointerMember<DensityMap> target_dens_map_;
  mutable IMP::PointerMember<SampledDensityMap> model_dens_map_;
  mutable SampledDensityMaps rb_model_dens_map_;
  mutable IMP::PointerMember<SampledDensityMap> none_rb_model_dens_map_;
  algebra::BoundingBoxD<3> target_bounding_box_;
  // reference to the IMP environment
  float scalefac_;
  core::XYZs xyz_;
  // derivatives
  mutable algebra::Vector3Ds dv_;
  algebra::ReferenceFrame3Ds rbs_orig_rf_;
  FloatKey weight_key_;
  KernelParameters *kernel_params_;
  FloatPair norm_factors_;
  bool use_rigid_bodies_;
  // particle handling
  // map particles to their rigid bodies
  boost::unordered_map<core::RigidBody, Particles> member_map_;
  Particles all_ps_;
  // all particles that are not part of a rigid body
  Particles not_part_of_rb_;
  Particles part_of_rb_;
  core::RigidBodies rbs_;
  KernelType kt_;
};

IMPEM_END_NAMESPACE

#endif /* IMPEM_FIT_RESTRAINT_H */
