/**
 *  \file Copy.cpp   \brief Simple xyz decorator.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/Copy.h>

IMPATOM_BEGIN_NAMESPACE

IntKey Copy::get_copy_index_key() {
  static IntKey k("copy index");
  return k;
}

int get_copy_index(Hierarchy h) {
  while (h) {
    if (Copy::get_is_setup(h)) {
      return Copy(h).get_copy_index();
    }
    h = h.get_parent();
  }
  return -1;
}

void Copy::show(std::ostream &out) const { out << get_copy_index(); }
IMPATOM_END_NAMESPACE
