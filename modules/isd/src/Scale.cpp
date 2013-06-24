/**
 *  \file Scale.h
 *  \brief A decorator for scale parameters particles
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/isd/Scale.h>

IMPISD_BEGIN_NAMESPACE

void Scale::do_setup_particle(Model *m, ParticleIndex pi, double scale)
{
  if (!Nuisance::get_is_setup(m, pi))
    Nuisance::setup_particle(m, pi,scale);
  Nuisance(m, pi).set_lower(0.);
}


void Scale::show(std::ostream &out) const {
  Particle * p = get_particle();
  bool hasupper = p->has_attribute(get_upper_key());
  out << get_lower() << " < ";
  out << " Scale = " << get_nuisance();
  if (hasupper) out << " < " << get_upper();
}

IMPISD_END_NAMESPACE
