/**
 *  \file atom/Charged.cpp
 *  \brief A decorator for a point particle that has an electrostatic charge.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/Charged.h"
#include <IMP/algebra/Vector3D.h>

IMPATOM_BEGIN_NAMESPACE

FloatKey Charged::get_charge_key() {
  static FloatKey k("charge");
  return k;
}

void Charged::show(std::ostream &out) const {
  XYZ::show(out);
  out << " charge= " << get_charge();
}

IMPATOM_END_NAMESPACE
