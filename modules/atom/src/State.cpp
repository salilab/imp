/**
 *  \file State.cpp   \brief Simple xyz decorator.
 *
 *  Stateright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/State.h>

IMPATOM_BEGIN_NAMESPACE

IntKey State::get_state_index_key() {
  static IntKey k("state index");
  return k;
}

void State::show(std::ostream &out) const { out << get_state_index(); }

IMPATOM_END_NAMESPACE
