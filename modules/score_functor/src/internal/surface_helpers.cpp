/**
 * \file surface_helpers.cpp
 * \brief Helper functions for surfaces.
 *
 * Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/score_functor/internal/surface_helpers.h>

IMPSCOREFUNCTOR_BEGIN_INTERNAL_NAMESPACE

FloatKey get_surface_normal_key(unsigned int i) {
  static const FloatKey surface_normal_keys[] = {FloatKey("normal_x"),
                                                 FloatKey("normal_y"),
                                                 FloatKey("normal_z")};
  return surface_normal_keys[i];
}

IMPSCOREFUNCTOR_END_INTERNAL_NAMESPACE
