/**
 *  \file core/utility.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_UTILITY_H
#define IMPCORE_UTILITY_H

#include "macros.h"
#include "config.h"
#include <IMP/base_types.h>
#include <IMP/algebra/Segment3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/Model.h>
#include <IMP/Particle.h>

IMPCORE_BEGIN_NAMESPACE
//! Get the centroid
IMPCOREEXPORT algebra::Vector3D centroid(const Particles &ps);

#ifndef IMP_NO_DEPRECATED
//! Transform a molecule
/**
   \deprecated see SingletonFunctor for a replacement
 */
IMPCOREEXPORT
void transform(const Particles &ps,
               const algebra::Transformation3D &t);
#endif // IMP_NO_DEPRECATED
IMPCOREEXPORT
algebra::Segment3D diameter(const Particles &ps);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_UTILITY_H */
