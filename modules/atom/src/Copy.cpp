/**
 *  \file Copy.cpp   \brief Simple xyz decorator.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/Copy.h>

IMPATOM_BEGIN_NAMESPACE

IntKey Copy::get_copy_index_key() {
  static IntKey k("molecule copy");
  return k;
}

void Copy::show(std::ostream &out) const { out << get_copy_index(); }
IMPATOM_END_NAMESPACE
