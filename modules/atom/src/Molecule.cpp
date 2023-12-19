/**
 *  \file Molecule.cpp   \brief A decorator for Molecules.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/Molecule.h>

#include <sstream>
#include <vector>

IMPATOM_BEGIN_NAMESPACE
IntKey Molecule::key() {
  static IntKey k("molecule tag");
  return k;
}

void Molecule::show(std::ostream &out) const {
  out << "name: " << get_particle()->get_name();
}

IMPATOM_END_NAMESPACE
