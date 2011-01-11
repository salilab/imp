/**
 *  \file cgal/internal/polyhedrons.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
*/

#ifndef IMPCGAL_INTERNAL_UNION_OF_BALLS_H
#define IMPCGAL_INTERNAL_UNION_OF_BALLS_H

#include "../cgal_config.h"
#include <IMP/base_types.h>
#include <IMP/algebra/SphereD.h>
#include <vector>


IMPCGAL_BEGIN_INTERNAL_NAMESPACE

IMPCGALEXPORT
std::pair<double, double>
get_surface_area_and_volume(const std::vector<algebra::SphereD<3> > &ss);

IMPCGAL_END_INTERNAL_NAMESPACE

#endif  /* IMPCGAL_INTERNAL_UNION_OF_BALLS_H */
