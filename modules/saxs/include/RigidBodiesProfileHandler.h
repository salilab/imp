/**
 * \file IMP/saxs/RigidBodiesProfileHandler.h
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_RIGID_BODIES_PROFILE_HANDLER_H
#define IMPSAXS_RIGID_BODIES_PROFILE_HANDLER_H

#include <IMP/saxs/saxs_config.h>
#include <IMP/saxs/Profile.h>
#include <IMP/saxs/DerivativeCalculator.h>

#include <IMP/core/rigid_bodies.h>

#include <IMP/Model.h>

#include <IMP/Object.h>
#include <IMP/Pointer.h>

IMPSAXS_BEGIN_NAMESPACE

//! Handle the profile for a set of particles, which may include rigid bodies
class IMPSAXSEXPORT RigidBodiesProfileHandler : public Object {
 public:
  RigidBodiesProfileHandler(const Particles& particles,
                            FormFactorType ff_type = HEAVY_ATOMS);

  void compute_profile(Profile* model_profile) const;

  // TODO: implement
  // void compute_profile_partial(Profile* model_profile) const;

  void compute_derivatives(const DerivativeCalculator* dc,
                           const Profile* model_profile,
                           const Vector<double>& effect_size,
                           DerivativeAccumulator* acc) const;

  ModelObjectsTemp do_get_inputs() const;

  IMP_OBJECT_METHODS(RigidBodiesProfileHandler);

 protected:
  Particles particles_;  // non-rigid bodies particles
  Vector<core::RigidBody> rigid_bodies_decorators_;  // rigid bodies
  Vector<Particles> rigid_bodies_;  // rigid bodies particles
  // non-changing part of the profile
  PointerMember<Profile> rigid_bodies_profile_;
  FormFactorType ff_type_;  // type of the form factors to use
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_RIGID_BODIES_PROFILE_HANDLER_H */
