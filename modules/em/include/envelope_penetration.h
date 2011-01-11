/**
 *  \file envelope_penetration.h
 *  \brief functions for calculation envelope penetration
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_ENVELOPE_PENETRATION_H
#define IMPEM_ENVELOPE_PENETRATION_H
#include "em_config.h"
#include "DensityMap.h"
#include <IMP/Particle.h>
IMPEM_BEGIN_NAMESPACE
//! Get the number of particles that are outside of the density
/**
/note the function assumes that all of the particles have XYZ coordinates
 */
IMPEMEXPORT long get_number_of_particles_outside_of_the_density(
     DensityMap *dmap,const Particles &ps,
     const IMP::algebra::Transformation3D &t=
         IMP::algebra::get_identity_transformation_3d(),float thr=0.0);
IMPEM_END_NAMESPACE

#endif  /* IMPEM_ENVELOPE_PENETRATION_H */
