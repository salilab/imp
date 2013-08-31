/**
 *  \file example/ExampleSingletonModifier.cpp
 *  \brief A singleton modifier which wraps an attribute into a
 *  given range.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/example/ExampleSingletonModifier.h"
#include "IMP/core/XYZ.h"

IMPEXAMPLE_BEGIN_NAMESPACE
ExampleSingletonModifier::ExampleSingletonModifier(
    const algebra::BoundingBox3D &bb)
    : bb_(bb) {}

void ExampleSingletonModifier::apply_index(Model *m,
                                           kernel::ParticleIndex pi) const {
  core::XYZ d(m, pi);
  for (unsigned int i = 0; i < 3; ++i) {
    // shift the coordinate until it is in the box
    while (d.get_coordinate(i) < bb_.get_corner(0)[i]) {
      d.set_coordinate(i, d.get_coordinate(i) +
                              (bb_.get_corner(1)[i] - bb_.get_corner(0)[i]));
    }
    while (d.get_coordinate(i) > bb_.get_corner(1)[i]) {
      d.set_coordinate(i, d.get_coordinate(i) -
                              (bb_.get_corner(1)[i] - bb_.get_corner(0)[i]));
    }
  }
}

/* Only the passed particle is used */
ModelObjectsTemp ExampleSingletonModifier::do_get_inputs(Model *m,
                                          const kernel::ParticleIndexes &pis) const {
  return IMP::kernel::get_particles(m, pis);
}

ModelObjectsTemp ExampleSingletonModifier::do_get_outputs(Model *m,
                                          const kernel::ParticleIndexes &pis) const {
  return do_get_inputs(m, pis);
}

IMPEXAMPLE_END_NAMESPACE
