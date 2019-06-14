/**
 *  \file TiltSingletonScore.cpp
 *  \brief A Score on the distance to a fixed point.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 */

#include <IMP/UnaryFunction.h>
#include <IMP/algebra.h>
#include <IMP/core.h>
#include <IMP/spb/TiltSingletonScore.h>

IMPSPB_BEGIN_NAMESPACE

TiltSingletonScore::TiltSingletonScore(UnaryFunction *f,
                                       const algebra::VectorD<3> &v1,
                                       const algebra::VectorD<3> &v2)
    : f_(f), local_(v1.get_unit_vector()), global_(v2.get_unit_vector()) {}

Float TiltSingletonScore::evaluate_index(IMP::Model *m,
                                         const IMP::ParticleIndex pi,
                                         DerivativeAccumulator *da) const {
  // check if derivatives are requested
  IMP_USAGE_CHECK(!da, "Derivatives not available");

  // check if rigid body
  IMP_USAGE_CHECK(core::RigidBody::get_is_setup(m->get_particle(pi)),
                  "Particle is not a rigid body");

  algebra::ReferenceFrame3D rf =
      core::RigidBody(m->get_particle(pi)).get_reference_frame();

  //std::cout << pi << " " << rf << std::endl;

  algebra::VectorD<3> local2global = rf.get_global_coordinates(local_);

  //std::cout << "local" << local_ << " " << local2global << std::endl;

  algebra::VectorD<3> origin =
      rf.get_global_coordinates(algebra::VectorD<3>(0.0, 0.0, 0.0));

  // std::cout << "origin" << origin << " " << global_ << std::endl ;
  //std::cout << local2global-origin << global_ <<std::endl;

  // now calculate the angle
  Float sp = std::max(
      -1.0, std::min(1.0, global_.get_scalar_product(local2global - origin)));
  Float angle = acos(sp);

  //std::cout << "ANGLE "<< angle <<std::endl;

  Float score = f_->evaluate(angle);

  //std::cout << "SCORE "<< score << std::endl;

  return score;
}

void TiltSingletonScore::show(std::ostream &out) const {
  out << "function " << *f_;
}

IMP::ModelObjectsTemp TiltSingletonScore::do_get_inputs(
    IMP::Model *m, const IMP::ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}

IMPSPB_END_NAMESPACE
