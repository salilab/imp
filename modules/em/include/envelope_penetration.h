/**
 *  \file IMP/em/envelope_penetration.h
 *  \brief functions for calculation envelope penetration
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_ENVELOPE_PENETRATION_H
#define IMPEM_ENVELOPE_PENETRATION_H
#include <IMP/em/em_config.h>
#include "DensityMap.h"
#include <IMP/kernel/Particle.h>

IMPEM_BEGIN_NAMESPACE
//! Get the number of particles that are outside of the density
/**
/note the function assumes that all of the particles have XYZ coordinates
 */
IMPEMEXPORT long get_number_of_particles_outside_of_the_density(
    DensityMap *dmap, const kernel::Particles &ps,
    const IMP::algebra::Transformation3D &t =
        IMP::algebra::get_identity_transformation_3d(),
    float thr = 0.0);
//! Get numbers of particles (mult transforms) that are outside the density
/**
/note the function assumes that all of the particles have XYZ coordinates
 */
IMPEMEXPORT Ints get_numbers_of_particles_outside_of_the_density(
    DensityMap *dmap, const kernel::Particles &ps,
    const IMP::algebra::Transformation3Ds &transformations, float thr = 0.0);
//! Get the number of density voxels that are not covered by particles
/**
/note the function assumes that all of the particles have XYZ coordinates
 */
IMPEMEXPORT double get_percentage_of_voxels_covered_by_particles(
    DensityMap *dmap, const kernel::Particles &ps, float smoothing_radius = 3.,
    const IMP::algebra::Transformation3D &t =
        IMP::algebra::get_identity_transformation_3d(),
    float thr = 0.0);
IMPEM_END_NAMESPACE

#endif /* IMPEM_ENVELOPE_PENETRATION_H */
