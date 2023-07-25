/**
 *  \file cgal/internal/union_of_balls.h
 *  \brief Utilities to handle multiple spheres.
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
*/

#ifndef IMPCGAL_INTERNAL_UNION_OF_BALLS_H
#define IMPCGAL_INTERNAL_UNION_OF_BALLS_H

#include <IMP/cgal/cgal_config.h>
#include <IMP/types.h>
#include <IMP/algebra/SphereD.h>
#include <vector>

IMPCGAL_BEGIN_INTERNAL_NAMESPACE

IMPCGALEXPORT std::pair<double, double> get_surface_area_and_volume(
    const algebra::Sphere3Ds &ss);

IMPCGAL_END_INTERNAL_NAMESPACE

#endif /* IMPCGAL_INTERNAL_UNION_OF_BALLS_H */
