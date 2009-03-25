/**
 *  \file core/utilities.h
 *  \brief Functions to perform simple functions on a set of particles
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPCORE_UTILITIES_H
#define IMPCORE_UTILITIES_H

#include "config.h"
#include <IMP/algebra/Transformation3D.h>
#include <IMP/algebra/Segment3D.h>
#include <IMP/Model.h>
#include <IMP/Particle.h>

IMPCORE_BEGIN_NAMESPACE

//! Get the centroid
IMPCOREEXPORT algebra::Vector3D centroid(const Particles &ps);
//! Transform a molecule
IMPCOREEXPORT
void transform(const Particles &ps,
               const algebra::Transformation3D &t);
IMPCOREEXPORT
algebra::Segment3D diameter(const Particles &ps);
IMPCORE_END_NAMESPACE

#endif /* IMPCORE_UTILITIES_H */
