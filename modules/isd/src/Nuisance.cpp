/**
 *  \file Nuisance.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/isd/Nuisance.h"

IMPISD_BEGIN_NAMESPACE

FloatKey Nuisance::get_nuisance_key() {
  static FloatKey k("nuisance");
  return k;
}

void Nuisance::show(std::ostream &out) const {
  out << "Nuisance " << get_nuisance() <<  std::endl;
}

IMPISD_END_NAMESPACE
