/**
 *  \file CoverBond.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/CoverBond.h"
#include <IMP/atom/bond_decorators.h>
#include <IMP/core/XYZR.h>

IMPATOM_BEGIN_NAMESPACE

CoverBond::CoverBond() {}

void CoverBond::apply_index(Model *m, kernel::ParticleIndex pi) const {
  Bond bd(m, pi);
  core::XYZ ea(bd.get_bonded(0)), eb(bd.get_bonded(1));
  core::XYZR r(m, pi);
  r.set_coordinates(.5 * (ea.get_coordinates() + eb.get_coordinates()));
  r.set_radius((r.get_coordinates() - ea.get_coordinates()).get_magnitude());
}

ModelObjectsTemp CoverBond::do_get_inputs(Model *m,
                                          const kernel::ParticleIndexes &pis) const {
  ModelObjectsTemp ret(3 * pis.size());
  for (unsigned int i = 0; i < pis.size(); ++i) {
    Bond bd(m, pis[i]);
    ret[3 * i + 0] = m->get_particle(pis[i]);
    ret[3 * i + 1] = bd.get_bonded(0);
    ret[3 * i + 2] = bd.get_bonded(1);
  }
  return ret;
}

ModelObjectsTemp CoverBond::do_get_outputs(Model *m,
                                           const kernel::ParticleIndexes &pis) const {
  return IMP::kernel::get_particles(m, pis);
}

IMPATOM_END_NAMESPACE
