/**
 *  \file core/helpers.h
 *  \brief Various helper functions for helping setting up restraints.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_HELPERS_H
#define IMPCORE_HELPERS_H

#include "config.h"
#include <IMP/base_types.h>
#include <IMP/Particle.h>

IMPCORE_BEGIN_NAMESPACE

//! Set up a diameter restraint for the set of XYZ particles
/** For large point sets, the restraint restrains the size of a
    bounding score containing the spheres or points. The sphere
    used is better if CGAL is used.

    \note This can be made more efficient two different ways
     - we can skip the cover for small point sets and just do all pairs
     - we can add a custom
 */
IMPCOREEXPORT void create_diameter_restraint(const Particles &ps,
                                             Float diameter,
                                             Float k);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_HELPERS_H */
