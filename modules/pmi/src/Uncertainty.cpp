/**
 *  \file Uncertainty.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/pmi/Uncertainty.h"

IMPPMI_BEGIN_NAMESPACE

FloatKey Uncertainty::get_uncertainty_key() {
  static FloatKey k("Uncertainty");
  return k;
}

void Uncertainty::show(std::ostream &out) const {
  out << "Uncertainty " << get_uncertainty() << std::endl;
}

namespace {
  bool check_Uncertainty(Model *m, ParticleIndex pi) {
    if (m->get_attribute(Uncertainty::get_uncertainty_key(), pi) < 0) {
    IMP_THROW("Uncertainty must be non-negative.", ValueException);
  }
  return true;
}
}

IMP_CHECK_DECORATOR(Uncertainty, check_Uncertainty);

IMPPMI_END_NAMESPACE
