/**
 *  \file Switching.cpp
 *  \brief defines a one-dimensional switching parameter (theta, ...)
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/Switching.h>

IMPISD_BEGIN_NAMESPACE

FloatKey Switching::get_switching_key() {
  return Nuisance::get_nuisance_key();
}

void Switching::show(std::ostream &out) const {
  out <<" Switching " << get_switching() <<  std::endl;
}

void Switching::set_switching(Float d) {
    Float d_;
    if (d < 0) {
        d_ = 0;
    } else if (d > 1) {
        d_ = 1;
    } else {
        d_ = d;
    }
    get_particle()->set_value(get_switching_key(), d_);
}

Switching Switching::setup_particle(Particle *p, double sw) {
    if (!Nuisance::particle_is_instance(p)) {
        Nuisance::setup_particle(p, sw);
    }
    if (0 > sw || 1 < sw ) 
        IMP_THROW("switching value out of bounds!", ModelException);
    return Switching(p);
};



IMPISD_END_NAMESPACE
