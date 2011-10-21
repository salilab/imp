/**
 *  \file Nuisance.cpp
 *  \brief defines a one-dimensional nuisance parameter (sigma, gamma, ...)
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/Nuisance.h>

IMPISD_BEGIN_NAMESPACE

FloatKey Nuisance::get_nuisance_key() {
  static FloatKey k("nuisance");
  return k;
}

FloatKey Nuisance::get_upper_key() {
  static FloatKey k("upper");
  return k;
}

FloatKey Nuisance::get_lower_key() {
  static FloatKey k("lower");
  return k;
}

void Nuisance::show(std::ostream &out) const {
  Particle * p = get_particle();
  bool haslower = p->has_attribute(get_lower_key());
  bool hasupper = p->has_attribute(get_upper_key());
  if (haslower) out << get_lower() << " < ";
  out << " Nuisance = " << get_nuisance();
  if (hasupper) out << " < " << get_upper();
}

void Nuisance::set_upper(Float d) {
    Particle * p = get_particle();
    if (! p->has_attribute(get_upper_key()))
            p->add_attribute(get_upper_key(), d);
    p->set_value(get_upper_key(), d);
}

void Nuisance::set_lower(Float d) {
    Particle * p = get_particle();
    if (! p->has_attribute(get_lower_key()))
            p->add_attribute(get_lower_key(), d);
    p->set_value(get_lower_key(), d);
}

void Nuisance::set_nuisance(Float d) {
    Float d_=d;
    Particle *p=get_particle();
    if (p->has_attribute(get_lower_key()))
    {
        Float lo = get_lower();
        if (d < lo)  d_ = lo;
    }
    if (p->has_attribute(get_upper_key()))
    {
        Float up = get_upper();
        if (d > up)  d_ = up;
    }
    p->set_value(get_nuisance_key(), d_);
}

Nuisance Nuisance::setup_particle(Particle *p, double nuisance) {
    if (!Nuisance::particle_is_instance(p))
    {
        p->add_attribute(get_nuisance_key(), nuisance);
    } else {
        Nuisance(p).set_nuisance(nuisance);
    }
    return Nuisance(p);
}

IMPISD_END_NAMESPACE
