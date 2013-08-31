/**
 *  \file Mass.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/Mass.h"

IMPATOM_BEGIN_NAMESPACE

FloatKey Mass::get_mass_key() {
  static FloatKey k("mass");
  return k;
}

void Mass::show(std::ostream &out) const {
  out << "Mass " << get_mass() << std::endl;
}

namespace {
  bool check_mass(Model *m, kernel::ParticleIndex pi) {
    if (m->get_attribute(Mass::get_mass_key(), pi) < 0) {
    IMP_THROW("Mass must be non-negative.", ValueException);
  }
  return true;
}
}

IMP_CHECK_DECORATOR(Mass, check_mass);

IMPATOM_END_NAMESPACE
