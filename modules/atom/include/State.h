/**
 *  \file IMP/atom/State.h     \brief A decorator for States.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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

IMPATOMEXPORT extern const int ALL_STATES;

//! A decorator for a node that has multiple states of the system.
/** It stores a number of copies of its sub hierarchy each with an associated
 * state.
 *
 * \note All the subtrees for the different states should have identical
 * topology. However this is not current enforced.
 *
 * \note The particle returned for a given state should be considered as
 * replacing this particle (as opposed to being a child of it).
 *
 * \note Only one State node is allowed in any path up the tree as nesting
 * them does not have a clear meaning.
 */
class IMPATOMEXPORT State : public Hierarchy {
  static ParticleIndexesKey get_states_key();
  static IntKey get_marker_key();

  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi);

  void validate();

 public:
  IMP_DECORATOR_SETUP_0(State);
  IMP_DECORATOR_METHODS(State, Hierarchy);

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return m->get_has_attribute(get_marker_key(), pi);
  }

  /** Return all children at all resolutions. */
  Hierarchies get_states() const;

  /** Add the state with the next index (returned).
   */
  unsigned int add_state(kernel::ParticleIndexAdaptor rep);
};

IMP_DECORATORS(State, States, Hierarchies);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_STATE_H */
