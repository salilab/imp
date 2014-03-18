/**
 *  \file RigidBodyTunneler.cpp
 *  \brief A mover that transforms a rigid body
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core/RigidBodyTunneler.h>
#include <IMP/base/random.h>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_01.hpp>

IMPCORE_BEGIN_NAMESPACE

RigidBodyTunneler::RigidBodyTunneler(kernel::Model *m, kernel::ParticleIndex pi,
        kernel::ParticleIndex ref, double move_probability)
    : MonteCarloMover(m, m->get_particle(pi)->get_name() + " tunneler"),
    pi_(pi), ref_(ref), move_probability_(move_probability)
{
  IMP_USAGE_CHECK(RigidBody(m, pi).get_coordinates_are_optimized(),
                  "Rigid body passed to RigidBodyTunneler"
                      << " must be set to be optimized. particle: "
                      << m->get_particle_name(pi));
  IMP_USAGE_CHECK(move_probability_ >=0 && move_probability <= 1,
          "Probability must be 0<= p <= 1");
}

MonteCarloMoverResult RigidBodyTunneler::do_propose() {
  IMP_OBJECT_LOG;
  //check whether mover is usable
  if (entries_.size() < 2)
      IMP_THROW("You must add at least two entry points for this "
              << "mover to be usable.", ValueException);
  RigidBody d(get_model(), pi_), ref(get_model(), ref_);
  IMP_USAGE_CHECK(
      d.get_coordinates_are_optimized(),
      "Rigid body passed to RigidBodyTunneler"
          << " must be set to be optimized. particle: " << d->get_name());
  //get current reference frame of rb
  last_transformation_ = d.get_reference_frame().get_transformation_to();
  const algebra::Vector3D com(last_transformation_.get_translation());
  const algebra::Vector3D refcom(
          ref.get_reference_frame().get_transformation_to().get_translation());
  //see if we are to do the move
  ::boost::uniform_01<double> rand01;
  if (rand01(base::random_number_generator) <= move_probability_) {
      //compute the closest entry point
      unsigned closest=0;
      double mindistsq;
      for (unsigned i=0; i<entries_.size(); i++) {
          const double distsq = ((entries_[i]-refcom)-com
                                ).get_squared_magnitude();
          if (i==0 || mindistsq > distsq){
              mindistsq = distsq;
              closest = i;
          }
      }
      //pick another entry point at random
      ::boost::uniform_int<unsigned> randint(0,entries_.size()-1);
      unsigned distant;
      do {
          distant = randint(base::random_number_generator);
      } while (distant == closest);
      //propose move to it
      algebra::Transformation3D t(last_transformation_.get_rotation(),
                                  entries_[distant]-entries_[closest]);
      IMP_LOG_VERBOSE("proposed move " << t << " from entry point " << closest 
              << " to " << distant << std::endl);
      d.set_reference_frame(algebra::ReferenceFrame3D(t));
  }
  return MonteCarloMoverResult(kernel::ParticleIndexes(1, pi_), 1.0);
}

void RigidBodyTunneler::do_reject() {
  RigidBody d(get_model(), pi_);
  d.set_reference_frame(algebra::ReferenceFrame3D(last_transformation_));
  last_transformation_ = algebra::Transformation3D();
}

kernel::ModelObjectsTemp RigidBodyTunneler::do_get_inputs() const {
  return kernel::ModelObjectsTemp(1, get_model()->get_particle(pi_));
}

IMPCORE_END_NAMESPACE
