/**
 *  \file Symmetric.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/pmi/Symmetric.h"

IMPPMI_BEGIN_NAMESPACE

FloatKey Symmetric::get_symmetric_key() {
  static FloatKey k("symmetric");
  return k;
}

void Symmetric::show(std::ostream &out) const {
  out << "Symmetric " << get_symmetric() << std::endl;
}

namespace {
  bool check_symmetric(Model *m, ParticleIndex pi) {
    if (m->get_attribute(Symmetric::get_symmetric_key(), pi) < 0) {
    IMP_THROW("Symmetric must be non-negative.", ValueException);
  }
  return true;
}
}

IMP_CHECK_DECORATOR(Symmetric, check_symmetric);

IMPPMI_END_NAMESPACE
