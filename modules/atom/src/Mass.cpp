/**
 *  \file Mass.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/Mass.h"


IMPATOM_BEGIN_NAMESPACE


FloatKey Mass::get_mass_key() {
  static FloatKey k("mass");
  return k;
}

void Mass::show(std::ostream &out) const {
  out << "Mass " << get_mass() <<  std::endl;
}

IMPATOM_END_NAMESPACE
