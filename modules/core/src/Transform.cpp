/**
 *  \file TransformSingletonParticle.cpp
 *  \brief Transform a particle
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/core/Transform.h>
#include "IMP/core/XYZ.h"

IMPCORE_BEGIN_NAMESPACE

Transform::Transform(const algebra::Transformation3D &t, bool ignore_non_xyz) {
  t_ = t;
  ignore_non_xyz_ = ignore_non_xyz;
}

void Transform::apply_index(Model *m, ParticleIndex pi) const {
  if (!XYZ::get_is_setup(m, pi)) {
    IMP_INTERNAL_CHECK(ignore_non_xyz_,
                       "The particle does not have XYZ attributes");
    return;
  }
  XYZ xyz = XYZ(m, pi);
  xyz.set_coordinates(t_.get_transformed(xyz.get_coordinates()));
}

ModelObjectsTemp Transform::do_get_inputs(Model *m,
                                          const ParticleIndexes &pis) const {
  return IMP::kernel::get_particles(m, pis);
}

ModelObjectsTemp Transform::do_get_outputs(Model *m,
                                           const ParticleIndexes &pis) const {
  return IMP::kernel::get_particles(m, pis);
}

IMPCORE_END_NAMESPACE
