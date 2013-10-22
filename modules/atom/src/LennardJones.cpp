/**
 *  \file atom/LennardJones.cpp
 *  \brief A decorator for a particle that has a Lennard-Jones potential well.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/LennardJones.h"

IMPATOM_BEGIN_NAMESPACE

FloatKey LennardJones::get_well_depth_key() {
  static FloatKey k("lennard_jones_well_depth");
  return k;
}

void LennardJones::show(std::ostream &out) const {
  XYZR::show(out);
  out << " Lennard-Jones well depth= " << get_well_depth();
}

IMPATOM_END_NAMESPACE
