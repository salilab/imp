/**
 *  \file State.cpp   \brief Simple xyz decorator.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/State.h>

IMPATOM_BEGIN_NAMESPACE

IntKey State::get_index_key() {
  static IntKey k("state index");
  return k;
}

void State::do_setup_particle(Model *m, ParticleIndex pi,
                              unsigned int state) {
  m->add_attribute(get_index_key(), pi, state);
}

void State::show(std::ostream &out) const { out << "State: " << get_state_index(); }

int get_state_index(Hierarchy h) {
  while (h) {
    if (State::get_is_setup(h)) {
      return State(h).get_state_index();
    }
    h = get_parent_representation(h);
  }
  return -1;
}

IMPATOM_END_NAMESPACE
