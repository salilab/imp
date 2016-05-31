/**
 *  \file IMP/atom/State.h     \brief A decorator for States.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_STATE_H
#define IMPATOM_STATE_H

#include <IMP/atom/atom_config.h>
#include "atom_macros.h"
#include "Hierarchy.h"

#include <IMP/base_types.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/Decorator.h>

IMPATOM_BEGIN_NAMESPACE

//! Associate an integer "state" index with a hierarchy node.
/**
 * \note Only one State node is allowed in any path up the tree as nesting
 * them does not have a clear meaning.
 */
class IMPATOMEXPORT State : public Hierarchy {
  static IntKey get_index_key();

  static void do_setup_particle(Model *m, ParticleIndex pi,
                                unsigned int state);
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                State o){
    do_setup_particle(m, pi, o.get_state_index());
  }
  void validate();

 public:
  IMP_DECORATOR_SETUP_1(State, unsigned int, index);
  IMP_DECORATOR_SETUP_1(State, State, other);
  IMP_DECORATOR_METHODS(State, Hierarchy);

  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_index_key(), pi);
  }

  unsigned int get_state_index() const {
    return get_model()->get_attribute(get_index_key(), get_particle_index());
  }
};

IMP_DECORATORS(State, States, Hierarchies);

//! Walk up the hierarchy to find the current state.
/** \return the state index, or -1 if there is none. */
IMPATOMEXPORT int get_state_index(Hierarchy h);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_STATE_H */
