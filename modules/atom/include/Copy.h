/**
 *  \file IMP/atom/Copy.h     \brief A particle with a user-defined type.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_COPY_H
#define IMPATOM_COPY_H

#include <IMP/atom/atom_config.h>
#include "../macros.h"

#include "Molecule.h"
#include <IMP/Decorator.h>
#include <vector>
#include <limits>

IMPATOM_BEGIN_NAMESPACE

//! A decorator for keeping track of copies of a molecule.
/** This decorator is for differentiating and keeping track
    of identity when there are multiple copies of the a
    molecule in the system. It should only be applied to
    Molecule particles.
 */
class IMPATOMEXPORT Copy : public Molecule {
 public:

  static IntKey get_copy_index_key();

  IMP_DECORATOR(Copy, Molecule);

  /** Create a decorator for the numberth copy. */
  static Copy setup_particle(Particle *p, int number) {
    p->add_attribute(get_copy_index_key(), number);
    Molecule::setup_particle(p);
    return Copy(p);
  }

  static bool particle_is_instance(Particle *p) {
    return p->has_attribute(get_copy_index_key());
  }

  int get_copy_index() const {
    return get_particle()->get_value(get_copy_index_key());
  }
};

IMP_DECORATORS(Copy, Copies, ParticlesTemp);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_COPY_H */
