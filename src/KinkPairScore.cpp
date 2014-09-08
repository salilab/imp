/**
 *  \file KinkPairScore.cpp
 *  \brief A score on the kink angle between two rigid bodies
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP/membrane/KinkPairScore.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/UnaryFunction.h>

IMPMEMBRANE_BEGIN_NAMESPACE

KinkPairScore::KinkPairScore(UnaryFunction *f): f_(f) {}
/*
Float KinkPairScore::evaluate(const ParticlePair &p,
                                 DerivativeAccumulator *da) const
{
  // check if derivatives are requested
  IMP_USAGE_CHECK(!da, "Derivatives not available");

  // check if rigid body
  IMP_USAGE_CHECK(core::RigidBody::get_is_setup(p[0]),
                  "Particle is not a rigid body");
  IMP_USAGE_CHECK(core::RigidBody::get_is_setup(p[1]),
                  "Particle is not a rigid body");

  // principal axis of inertia is aligned to x axis when creating rigid body
  algebra::Vector3D inertia=algebra::Vector3D(1.0,0.0,0.0);
  algebra::Vector3D  origin=algebra::Vector3D(0.0,0.0,0.0);

  // get the two references frames
  algebra::ReferenceFrame3D rf0 = core::RigidBody(p[0]).get_reference_frame();
  algebra::ReferenceFrame3D rf1 = core::RigidBody(p[1]).get_reference_frame();

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
*/
Float KinkPairScore::evaluate_index(kernel::Model *m,
const kernel::ParticleIndexPair &pip,
DerivativeAccumulator *da) const
{
  // check if derivatives are requested
  IMP_USAGE_CHECK(!da, "Derivatives not available");

  // check if rigid body
  IMP_USAGE_CHECK(core::RigidBody::get_is_setup(
     m->get_particle(pip[0])),"Particle is not a rigid body");
  IMP_USAGE_CHECK(core::RigidBody::get_is_setup(
     m->get_particle(pip[1])),"Particle is not a rigid body");

  // principal axis of inertia is aligned to x axis when creating rigid body
  algebra::Vector3D inertia=algebra::Vector3D(1.0,0.0,0.0);
  algebra::Vector3D  origin=algebra::Vector3D(0.0,0.0,0.0);

  // get the two references frames
  algebra::ReferenceFrame3D rf0 = core::RigidBody(
            m->get_particle(pip[0])).get_reference_frame();
  algebra::ReferenceFrame3D rf1 = core::RigidBody(
            m->get_particle(pip[1])).get_reference_frame();

  //rigid body 0
  algebra::Vector3D i0 = rf0.get_global_coordinates(inertia);
  algebra::Vector3D o0 = rf0.get_global_coordinates(origin);

  //rigid body 1
  algebra::Vector3D i1 = rf1.get_global_coordinates(inertia);
  algebra::Vector3D o1 = rf1.get_global_coordinates(origin);

  //now calculate the angle
  Float sp=std::max(-1.0,std::min(1.0,(i1-o1).get_scalar_product(i0-o0)));
  Float angle = acos(sp);

  //std::cout << "ANGLE "<< angle <<std::endl;

  Float score = f_->evaluate(angle);

  return score;

}


kernel::ModelObjectsTemp KinkPairScore::do_get_inputs(kernel::Model *m,
   const kernel::ParticleIndexes &pis)
    const {
  // return any particles that would be read if p is one of the particles
  // being scored. Don't worry about returning duplicates.
  return kernel::get_particles(m, pis);
}

/*
ContainersTemp KinkPairScore::get_input_containers(Particle *p) const {
  // return any containers that would be read if p is one of the particles
  // being scored. Don't worry about returning duplicates.
  return ContainersTemp();
}
*/
void KinkPairScore::show(std::ostream &out) const
{
 out << "function " << *f_;
}

IMPMEMBRANE_END_NAMESPACE
