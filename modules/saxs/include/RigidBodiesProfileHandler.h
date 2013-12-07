/**
 * \file saxs/RigidBodiesProfileHandler.h
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_RIGID_BODIES_PROFILE_HANDLER_H
#define IMPSAXS_RIGID_BODIES_PROFILE_HANDLER_H

#include <IMP/saxs/saxs_config.h>
#include <IMP/saxs/Profile.h>
#include <IMP/saxs/DerivativeCalculator.h>

#include <IMP/core/rigid_bodies.h>

#include <IMP/kernel/Model.h>

#include <IMP/base/Object.h>
#include <IMP/base/Pointer.h>

IMPSAXS_BEGIN_NAMESPACE

class IMPSAXSEXPORT RigidBodiesProfileHandler : public base::Object {
 public:
  RigidBodiesProfileHandler(const kernel::Particles& particles,
                            FormFactorType ff_type = HEAVY_ATOMS);

  void compute_profile(Profile* model_profile) const;

  // TODO: implement
  // void compute_profile_partial(Profile* model_profile) const;

  void compute_derivatives(const DerivativeCalculator* dc,
                           const Profile* model_profile,
                           const std::vector<double>& effect_size,
                           DerivativeAccumulator* acc) const;

  ModelObjectsTemp do_get_inputs() const;

  IMP_OBJECT_METHODS(RigidBodiesProfileHandler);

 protected:
  kernel::Particles particles_;  // non-rigid bodies particles
  std::vector<core::RigidBody> rigid_bodies_decorators_;  // rigid bodies
  std::vector<kernel::Particles> rigid_bodies_;  // rigid bodies particles
  // non-changing part of the profile
  base::PointerMember<Profile> rigid_bodies_profile_;
  FormFactorType ff_type_;  // type of the form factors to use
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_RIGID_BODIES_PROFILE_HANDLER_H */
