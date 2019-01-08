/**
 * \file direction_helpers.cpp
 * \brief Helper functions for directions.
 *
 * Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/score_functor/internal/direction_helpers.h>

IMPSCOREFUNCTOR_BEGIN_INTERNAL_NAMESPACE

FloatKey get_direction_key(unsigned int i) {
  static const FloatKey direction_keys[] = {FloatKey("direction_x"),
                                            FloatKey("direction_y"),
                                            FloatKey("direction_z")};
  return direction_keys[i];
}

algebra::Vector3D get_direction(Model *m, ParticleIndex pi) {
  return algebra::Vector3D(m->get_attribute(get_direction_key(0), pi),
                           m->get_attribute(get_direction_key(1), pi),
                           m->get_attribute(get_direction_key(2), pi));
}

IMPSCOREFUNCTOR_END_INTERNAL_NAMESPACE
