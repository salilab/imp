/**
 *  \file NOEPairScore.cpp
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP/isd/NOEPairScore.h>
#include <IMP/isd/FNormal.h>
#include <IMP/isd/Nuisance.h>
#include <IMP/core/XYZ.h>
#include <IMP/UnaryFunction.h>
#include <math.h>


IMPISD_BEGIN_NAMESPACE

NOEPairScore::NOEPairScore(Particle * sigma, Particle * gamma): 
  sigma_(sigma), gamma_(gamma) {}


Float NOEPairScore::evaluate(const ParticlePair &p,
                                 DerivativeAccumulator *da) const
{
  // assume they have coordinates
  core::XYZ d0(p[0]);
  core::XYZ d1(p[1]);
  double diff = (d0.get_coordinates()-d1.get_coordinates()).get_magnitude();
  double gamma_val=Nuisance(gamma_).get_nuisance();
  double Icalc = gamma_val*pow(diff,-6);
  double sigma_val=Nuisance(sigma_).get_nuisance();
  double Fx = log(Iexp_);
  double Fu = log(Icalc);
  double Jx = 1.0/Iexp_;
  IMP_NEW(FNormal, lognormal, (sigma_val,Fx,Fu,Jx)); 
  double score= lognormal->evaluate(0);
  if (da) {
    // THESE ARE INCORRECT!!!!!!
    // derivatives are requested
    algebra::Vector3D delta= d0.get_coordinates()-d1.get_coordinates();
    algebra::Vector3D udelta= delta.get_unit_vector();
    double dv= diff;
    // add to the particle derivatives
    d0.add_to_derivatives(udelta*dv, *da);
    d1.add_to_derivatives(-udelta*dv, *da);
  }
  return score;
}

void NOEPairScore::set_Iexp(double f) {
  Iexp_=f;
}

bool NOEPairScore::get_is_changed(const ParticlePair &pp) const {
  // return whether the score may have changed since last invocation
  return pp[0]->get_is_changed() || pp[1]->get_is_changed();
}
ParticlesTemp NOEPairScore::get_input_particles(Particle *p) const {
  // return any particles that would be read if p is one of the particles
  // being scored. Don't worry about returning duplicates.
  return ParticlesTemp(1,p);
}
ContainersTemp NOEPairScore::get_input_containers(Particle *p) const {
  // return any containers that would be read if p is one of the particles
  // being scored. Don't worry about returning duplicates.
  return ContainersTemp();
}

void NOEPairScore::do_show(std::ostream &out) const {
  out << "sigma=" << sigma_ << " and gamma=" << gamma_ << std::endl;
}

IMPISD_END_NAMESPACE
