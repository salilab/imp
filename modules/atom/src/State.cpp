/**
 *  \file Atom.cpp   \brief Simple atoms decorator.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/State.h>
#include <IMP/base/log.h>

#include <sstream>
#include <vector>

IMPATOM_BEGIN_NAMESPACE

const int ALL_STATES = -1;

kernel::ParticleIndexesKey State::get_states_key() {
  static kernel::ParticleIndexesKey key("states");
  return key;
}

IntKey State::get_marker_key() {
  static kernel::IntKey key("is_states");
  return key;
}

void State::do_setup_particle(kernel::Model* m,
                                       kernel::ParticleIndex pi) {
  m->add_attribute(get_marker_key(), pi, 0);
}

Hierarchies State::get_states() const {
  Hierarchies ret(1, *this);
  if (get_model()->get_has_attribute(get_states_key(), get_particle_index())) {
    IMP_FOREACH(
        kernel::ParticleIndex pi,
        get_model()->get_attribute(get_states_key(), get_particle_index())) {
      ret.push_back(Hierarchy(get_model(), pi));
    }
  }
  return ret;
}

unsigned int State::add_state(ParticleIndexAdaptor rep) {
  // fake the parent
  if (get_model()->get_has_attribute(Hierarchy::get_traits().get_parent_key(),
                                     get_particle_index())) {
    get_model()->add_attribute(Hierarchy::get_traits().get_parent_key(), rep,
                               get_parent().get_particle_index());
  }
  if (get_model()->get_has_attribute(get_states_key(), get_particle_index())) {
    get_model()
        ->access_attribute(get_states_key(), get_particle_index())
        .push_back(rep);
    return get_model()
        ->get_attribute(get_states_key(), get_particle_index())
        .size();
  } else {
    get_model()->add_attribute(get_states_key(), get_particle_index(),
                               ParticleIndexes(1, rep));
    return 1;
  }
}

void State::show(std::ostream& out) const {
  out << get_states().size() << " states";
}

IMPATOM_END_NAMESPACE
