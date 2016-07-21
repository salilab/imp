/**
 *  \file RigidBodyAnglePairScore.cpp
 *  \brief A score on the kink angle between two rigid bodies
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 */

#include <IMP/core/rigid_bodies.h>
#include <IMP/core/RigidBodyAnglePairScore.h>
#include <IMP/algebra.h>
#include <IMP/UnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE

RigidBodyAnglePairScore::RigidBodyAnglePairScore(UnaryFunction *f): f_(f) {}

Float RigidBodyAnglePairScore::evaluate_index(Model *m,
                                        const ParticleIndexPair &pi,
                                        DerivativeAccumulator *da) const {
  IMP_UNUSED(da);
  // check if derivatives are requested
  IMP_USAGE_CHECK(!da, "Derivatives not implemented");

  // check if rigid body
  IMP_USAGE_CHECK(RigidBody::get_is_setup(m, pi[0]),
                  "Particle is not a rigid body");
  IMP_USAGE_CHECK(RigidBody::get_is_setup(m, pi[1]),
                  "Particle is not a rigid body");

  // principal axis of inertia is aligned to x axis when creating rigid body
  algebra::Vector3D inertia=algebra::Vector3D(1.0,0.0,0.0);
  algebra::Vector3D  origin=algebra::Vector3D(0.0,0.0,0.0);

  // get the two references frames
  algebra::ReferenceFrame3D rf0 = RigidBody(m, pi[0]).get_reference_frame();
  algebra::ReferenceFrame3D rf1 = RigidBody(m, pi[1]).get_reference_frame();

  // rigid body 0
  algebra::Vector3D i0 = rf0.get_global_coordinates(inertia);
  algebra::Vector3D o0 = rf0.get_global_coordinates(origin);

  // rigid body 1
  algebra::Vector3D i1 = rf1.get_global_coordinates(inertia);
  algebra::Vector3D o1 = rf1.get_global_coordinates(origin);

  // now calculate the angle
  Float sp=std::max(-1.0,std::min(1.0,(i1-o1).get_scalar_product(i0-o0)));
  Float angle = acos(sp);

  //std::cout << "ANGLE "<< angle <<std::endl;

  Float score = f_->evaluate(angle);

  return score;
}

ModelObjectsTemp RigidBodyAnglePairScore::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}

IMPCORE_END_NAMESPACE
