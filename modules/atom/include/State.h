/**
 *  \file IMP/atom/State.h     \brief A decorator for States.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_STATE_H
#define IMPATOM_STATE_H

#include <IMP/atom/atom_config.h>
#include "atom_macros.h"
#include "Hierarchy.h"

#include <IMP/base_types.h>
#include <IMP/kernel/Particle.h>
#include <IMP/kernel/Model.h>
#include <IMP/Decorator.h>

IMPATOM_BEGIN_NAMESPACE

//! Associate an integer "state" index with a hierarchy node.
/**
 * \note Only one State node is allowed in any path up the tree as nesting
 * them does not have a clear meaning.
 */
class IMPATOMEXPORT State : public Hierarchy {
  static IntKey get_index_key();

  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                unsigned int state);

  void validate();

 public:
  IMP_DECORATOR_SETUP_1(State, unsigned int, index);
  IMP_DECORATOR_METHODS(State, Hierarchy);

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return m->get_has_attribute(get_index_key(), pi);
  }

  unsigned int get_state_index() const {
    return get_model()->get_attribute(get_index_key(), get_particle_index());
  }
};

IMP_DECORATORS(State, States, Hierarchies);

/** Walk up the hierarchy to find the current state. Return -1 if there is none.
 */
IMPATOMEXPORT int get_state_index(Hierarchy h);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_STATE_H */
