/**
 *  \file DerivativesToRefined.cpp
 *  \brief Cover a bond with a sphere.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include "IMP/core/DerivativesToRefined.h"

#include "IMP/core/XYZ.h"

IMPCORE_BEGIN_NAMESPACE

DerivativesToRefined
::DerivativesToRefined(Refiner *r,
                                          FloatKeys ks): r_(r), ks_(ks)
{
}


void DerivativesToRefined
::apply(Particle *p,
        DerivativeAccumulator &da) const
{
  Particles ps = r_->get_refined(p);

  for (unsigned int i=0; i< ps.size(); ++i) {
    for (unsigned int j=0; j< ks_.size(); ++j) {
      Float f= p->get_derivative(ks_[j]);
      ps[i]->add_to_derivative(ks_[j], f, da);
    }
  }
}

ParticlesList
DerivativesToRefined::get_interacting_particles(Particle*p) const {
  ParticlesTemp pt=get_read_particles(p);
  pt.push_back(get_write_particles(p)[0]);
  return ParticlesList(1, pt);
}


ParticlesTemp
DerivativesToRefined::get_read_particles(Particle*p) const {
  return ParticlesTemp(1, p);
}

ParticlesTemp
DerivativesToRefined::get_write_particles(Particle*p) const {
  ParticlesTemp ps = r_->get_refined(p);
  return ps;
}

void DerivativesToRefined::show(std::ostream &out) const
{
  out << "DerivativesToRefined" << std::endl;
}

IMPCORE_END_NAMESPACE
