/**
 *  \file ExamplePairScore.cpp
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/example/ExamplePairScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/UnaryFunction.h>

#include <boost/lambda/lambda.hpp>

IMPEXAMPLE_BEGIN_NAMESPACE

ExamplePairScore::ExamplePairScore(double x0, double k): x0_(x0), k_(k) {}


Float ExamplePairScore::evaluate(const ParticlePair &p,
                                 DerivativeAccumulator *da) const
{
  // turn on logging for this method
  IMP_OBJECT_LOG;
  // assume they have coordinates
  core::XYZ d0(p[0]);
  core::XYZ d1(p[1]);
  // log something
  double diff = (d0.get_coordinates()-d1.get_coordinates()).get_magnitude()-x0_;
  IMP_LOG_VERBOSE( "The distance off from x0 is " << diff << std::endl);
  double score= .5*k_*square(diff);
  if (da) {
    // derivatives are requested
    algebra::Vector3D delta= d0.get_coordinates()-d1.get_coordinates();
    algebra::Vector3D udelta= delta.get_unit_vector();
    double dv= k_*diff;
    // add to the particle derivatives
    d0.add_to_derivatives(udelta*dv, *da);
    d1.add_to_derivatives(-udelta*dv, *da);
  }
  return score;
}


ParticlesTemp ExamplePairScore::get_input_particles(Particle *p) const {
  // return any particles that would be read if p is one of the particles
  // being scored. Don't worry about returning duplicates.
  return ParticlesTemp(1,p);
}
ContainersTemp ExamplePairScore::get_input_containers(Particle *) const {
  // return any containers that would be read if p is one of the particles
  // being scored. Don't worry about returning duplicates.
  return ContainersTemp();
}


void ExamplePairScore::do_show(std::ostream &out) const {
  out << "x0=" << x0_ << " and k=" << k_ << std::endl;
}

IMPEXAMPLE_END_NAMESPACE
