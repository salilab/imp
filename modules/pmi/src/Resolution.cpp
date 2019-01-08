/**
 *  \file Resolution.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/pmi/Resolution.h"

IMPPMI_BEGIN_NAMESPACE

FloatKey Resolution::get_resolution_key() {
  static FloatKey k("resolution");
  return k;
}

void Resolution::show(std::ostream &out) const {
  out << "Resolution " << get_resolution() << std::endl;
}

namespace {
  bool check_resolution(Model *m, ParticleIndex pi) {
    if (m->get_attribute(Resolution::get_resolution_key(), pi) < 0) {
    IMP_THROW("Resolution must be non-negative.", ValueException);
  }
  return true;
}
}

IMP_CHECK_DECORATOR(Resolution, check_resolution);

IMPPMI_END_NAMESPACE
