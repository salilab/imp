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
  return ParticlesList(1, get_used_particles(p));
}


ParticlesTemp
DerivativesToRefined::get_used_particles(Particle*p) const {
  ParticlesTemp t(1, p);
  ParticlesTemp ps = r_->get_refined(p);
  t.insert(t.end(), ps.begin(), ps.end());
  return t;
}

void DerivativesToRefined::show(std::ostream &out) const
{
  out << "DerivativesToRefined" << std::endl;
}

IMPCORE_END_NAMESPACE
