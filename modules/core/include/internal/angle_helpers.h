/**
 *  \file anglel_helpers.h
 *  \brief Helper functions to calculate and handle angles
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_ANGLE_HELPERS_H
#define IMPCORE_ANGLE_HELPERS_H

#include "../config.h"
#include "IMP/constants.h"

#include <cmath>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

//! Return the difference between two angles
/** Both angles, and the difference, are given in radians.
    The difference is the shortest distance from angle1 to angle2, and is
    thus in the range -PI to PI.
 */
double get_angle_difference(double angle1, double angle2)
{
  angle1 = std::fmod(angle1, 2.0 * PI);
  angle2 = std::fmod(angle2, 2.0 * PI);
  double diff = angle2 - angle1;
  return diff < -PI ? diff + 2.0 * PI : diff > PI ? diff - 2.0 * PI : diff;
}

IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_ANGLE_HELPERS_H */
