/**
 *  \file CommonEndpointPairFilter.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/misc/CommonEndpointPairFilter.h"
#include <IMP/atom/bond_decorators.h>

IMPMISC_BEGIN_NAMESPACE

CommonEndpointPairFilter::CommonEndpointPairFilter() {}

int CommonEndpointPairFilter::get_value_index(
    Model *m, const ParticleIndexPair &p) const {
  if (!IMP::atom::Bond::get_is_setup(m, p[0]) ||
      !IMP::atom::Bond::get_is_setup(m, p[1])) {
    return false;
  } else {
    IMP::atom::Bond b0(m, p[0]);
    IMP::atom::Bond b1(m, p[1]);
    return b0.get_bonded(0) == b1.get_bonded(1) ||
           b0.get_bonded(1) == b1.get_bonded(0) ||
           b0.get_bonded(0) == b1.get_bonded(0) ||
           b0.get_bonded(1) == b1.get_bonded(1);
  }
}

ModelObjectsTemp CommonEndpointPairFilter::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret = IMP::get_particles(m, pis);
  for (unsigned int i = 0; i < pis.size(); ++i) {
    if (IMP::atom::Bond::get_is_setup(m, pis[i])) {
      IMP::atom::Bond b(m, pis[i]);
      ret.push_back(b.get_bonded(0));
      ret.push_back(b.get_bonded(1));
    }
  }
  return ret;
}

IMPMISC_END_NAMESPACE
