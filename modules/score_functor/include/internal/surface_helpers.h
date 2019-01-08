/**
 * \file surface_helpers.h
 * \brief Helper functions for surfaces.
 *
 * Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSCORE_FUNCTOR_SURFACE_HELPERS_H
#define IMPSCORE_FUNCTOR_SURFACE_HELPERS_H

#include <IMP/score_functor/score_functor_config.h>
#include <IMP/algebra/VectorD.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>

IMPSCOREFUNCTOR_BEGIN_INTERNAL_NAMESPACE

//! Get height of point above surface.
/** \return the height above the surface
    If derv is non-nullptr, it is filled with the direction of
    increasing height of the point.
*/
IMPSCOREFUNCTOREXPORT double get_height_above_surface(
    const algebra::Vector3D &center,
    const algebra::Vector3D &normal,
    const algebra::Vector3D &point,
    algebra::Vector3D *derv);

//! Get depth of point below surface.
/** \return the depth below the surface
    If derv is non-nullptr, it is filled with the direction of
    increasing depth of the point.
*/
IMPSCOREFUNCTOREXPORT double get_depth_below_surface(
    const algebra::Vector3D &center,
    const algebra::Vector3D &normal,
    const algebra::Vector3D &point,
    algebra::Vector3D *derv);

//! Get distance of point from surface.
/** \return the distance from the surface
    If derv is non-nullptr, it is filled with the direction of
    increasing distance of the point.
*/
IMPSCOREFUNCTOREXPORT double get_distance_from_surface(
    const algebra::Vector3D &center,
    const algebra::Vector3D &normal,
    const algebra::Vector3D &point,
    algebra::Vector3D *derv);

IMPSCOREFUNCTOR_END_INTERNAL_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_SURFACE_HELPERS_H */
