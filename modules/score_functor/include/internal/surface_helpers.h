/**
 * \file surface_helpers.h
 * \brief Helper functions for surfaces.
 *
 * Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSCORE_FUNCTOR_SURFACE_HELPERS_H
#define IMPSCORE_FUNCTOR_SURFACE_HELPERS_H

#include <IMP/score_functor/score_functor_config.h>
#include <IMP/algebra/VectorD.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>

IMPSCOREFUNCTOR_BEGIN_INTERNAL_NAMESPACE

//! Get key for surface normal attribute at coordinate i.
IMPSCOREFUNCTOREXPORT FloatKey get_surface_normal_key(unsigned int i);

//! Get key for surface normal vector for surface particle.
IMPSCOREFUNCTOREXPORT algebra::Vector3D get_surface_normal(Model *m,
                                                           ParticleIndex pi);

//! Get height of point above surface.
/** \return the height above the surface
    If dervc is non-nullptr, the two algebra::Vector3D parameters are filled
    with the derivatives with respect to the surface center and point.
*/
IMPSCOREFUNCTOREXPORT double get_height_above_surface(
    const algebra::Vector3D &center,
    const algebra::Vector3D &normal,
    const algebra::Vector3D &point,
    algebra::Vector3D *dervc,
    algebra::Vector3D *dervp);

//! Get depth of point below surface.
/** \return the depth below the surface
    If dervc is non-nullptr, the two algebra::Vector3D parameters are filled
    with the derivatives with respect to the surface center and point.
*/
IMPSCOREFUNCTOREXPORT double get_depth_below_surface(
    const algebra::Vector3D &center,
    const algebra::Vector3D &normal,
    const algebra::Vector3D &point,
    algebra::Vector3D *dervc,
    algebra::Vector3D *dervp);

//! Get distance of point from surface.
/** \return the distance from the surface
    If dervc is non-nullptr, the two algebra::Vector3D parameters are filled
    with the derivatives with respect to the surface center and point.
*/
IMPSCOREFUNCTOREXPORT double get_distance_from_surface(
    const algebra::Vector3D &center,
    const algebra::Vector3D &normal,
    const algebra::Vector3D &point,
    algebra::Vector3D *dervc,
    algebra::Vector3D *dervp);

IMPSCOREFUNCTOR_END_INTERNAL_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_SURFACE_HELPERS_H */
