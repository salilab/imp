/**
 *  \file TiltSingletonScore.cpp
 *  \brief A Score on the distance to a fixed point.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#include <IMP/membrane/TiltSingletonScore.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/UnaryFunction.h>

IMPMEMBRANE_BEGIN_NAMESPACE


TiltSingletonScore::TiltSingletonScore(UnaryFunction *f,
                                       const algebra::VectorD<3> &v1,
                                       const algebra::VectorD<3> &v2)
    : f_(f), local_(v1.get_unit_vector()), global_(v2.get_unit_vector()){}


Float TiltSingletonScore::evaluate(Particle *b,
                                         DerivativeAccumulator *da) const
{

  // check if derivatives are requested
  IMP_USAGE_CHECK(!da, "Derivatives not available");

  // check if rigid body
  IMP_USAGE_CHECK(core::RigidBody::particle_is_instance(b),
                  "Particle is not a rigid body");

  algebra::ReferenceFrame3D rf = core::RigidBody(b).get_reference_frame();

  algebra::VectorD<3> local2global
   = rf.get_global_coordinates(local_);

  algebra::VectorD<3> origin
   = rf.get_global_coordinates(algebra::VectorD<3>(0.0,0.0,0.0));

  //std::cout << local2global << global_ <<std::endl;

  // now calculate the angle
  Float angle
   = acos(global_.get_scalar_product(local2global-origin));

  //std::cout << "ANGLE "<< angle <<std::endl;

  Float score = f_->evaluate(angle);

  //std::cout << "SCORE "<< score << std::endl;

  return score;
}

void TiltSingletonScore::do_show(std::ostream &out) const
{
  out << "function " << *f_;
}


IMPMEMBRANE_END_NAMESPACE
