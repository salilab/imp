/**
 *  \file Scale.h
 *  \brief A decorator for scale parameters particles
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP/isd/Scale.h>

IMPISD_BEGIN_NAMESPACE

  Scale Scale::setup_particle(Particle *p, double scale)
  {
      if (!Nuisance::particle_is_instance(p))
          Nuisance::setup_particle(p,scale);
      Nuisance(p).set_lower(0);
      return Scale(p);
  }


void Scale::show(std::ostream &out) const {
  Particle * p = get_particle();
  bool hasupper = p->has_attribute(get_upper_key());
  out << get_lower() << " < ";
  out << " Scale = " << get_nuisance();
  if (hasupper) out << " < " << get_upper();
}

IMPISD_END_NAMESPACE

