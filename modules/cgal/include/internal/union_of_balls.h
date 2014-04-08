/**
 *  \file cgal/internal/polyhedrons.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
*/

#ifndef IMPCGAL_INTERNAL_UNION_OF_BALLS_H
#define IMPCGAL_INTERNAL_UNION_OF_BALLS_H

#include <IMP/cgal/cgal_config.h>
#include <IMP/base/types.h>
#include <IMP/algebra/SphereD.h>
#include <vector>

IMPCGAL_BEGIN_INTERNAL_NAMESPACE

IMPCGALEXPORT std::pair<double, double> get_surface_area_and_volume(
    const algebra::Sphere3Ds &ss);

IMPCGAL_END_INTERNAL_NAMESPACE

#endif /* IMPCGAL_INTERNAL_UNION_OF_BALLS_H */
