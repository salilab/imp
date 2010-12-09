/**
 *  \file Nuisance.h
 *  \brief A decorator for nuisance parameters particles
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */


#ifndef IMPISD_NUISANCE_H
#define IMPISD_NUISANCE_H

#include "isd_config.h"

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Decorator.h>

IMPISD_BEGIN_NAMESPACE

//! Add nuisance parameter to particle
/** The value of the nuisance parameter may express data
    or theory uncertainty.
 */
class IMPISDEXPORT Nuisance: public Decorator
{
public:
  static Nuisance setup_particle(Particle *p, double nuisance) {
    p->add_attribute(get_nuisance_key(), nuisance);
    return Nuisance(p);
  }

  static bool particle_is_instance(Particle *p) {
    return p->has_attribute(get_nuisance_key());
  }

  Float get_nuisance() const {
    return get_particle()->get_value(get_nuisance_key());
  }

  void set_nuisance(Float d) {
    get_particle()->set_value(get_nuisance_key(), d);
  }

  IMP_DECORATOR(Nuisance, Decorator);

  static FloatKey get_nuisance_key();
};

IMP_OUTPUT_OPERATOR(Nuisance);

typedef Decorators<Nuisance, Particles> Nuisances;

IMPISD_END_NAMESPACE

#endif  /* IMPISD_NUISANCE_H */
