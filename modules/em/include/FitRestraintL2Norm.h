/**
 *  \file IMP/em/FitRestraintL2Norm.h
 *  \brief Calculate score based on fit to EM map.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_FIT_RESTRAINT_L2_NORM_H
#define IMPEM_FIT_RESTRAINT_L2_NORM_H

#include <IMP/em/em_config.h>
#include "DensityMap.h"
#include "CoarseL2Norm.h"
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

class IMPEMEXPORT FitRestraintL2Norm : public Restraint {
 public:
  //! Constructor
  FitRestraintL2Norm(ParticlesTemp ps,
		    DensityMap *em_map,
		    FloatKey weight_key = atom::Mass::get_mass_key(),
		     bool use_rigid_bodies = true, double sigma = .1);
  
  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
    const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(FitRestraintL2Norm);

#ifndef SWIG
  IMP_LIST(private, Particle, particle, Particle *, Particles);
#endif
 private:
  //! Store particles
  void store_particles(ParticlesTemp ps);

  //! Create density maps: one for each rigid body and one for the rest.
  void initialize_rigid_body(FloatKey weight_key);

  IMP::PointerMember<DensityMap> target_dens_map_;
  double resolution_;
  double voxel_size_;
  double sigma_;
  algebra::BoundingBoxD<3> target_bounding_box_;
  // reference to the IMP environment
  core::XYZs xyz_;

  // score and derivatives
  double score_;
  algebra::Vector3Ds dv_;
  algebra::ReferenceFrame3Ds rbs_orig_rf_;
  FloatKey weight_key_;
  KernelParameters *kernel_params_;
  bool use_rigid_bodies_;

  // particle handling
  // map particles to their rigid bodies
  boost::unordered_map<core::RigidBody, Particles> member_map_;
  Particles all_ps_;
  // all particles that are not part of a rigid body
  Particles not_part_of_rb_;
  Particles part_of_rb_;
  core::RigidBodies rbs_;
};

IMPEM_END_NAMESPACE

#endif /* IMPEM_FIT_RESTRAINT_L2_NORM_H */
