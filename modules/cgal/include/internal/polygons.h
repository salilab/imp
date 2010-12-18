/**
 *  \file cgal/internal/polygons.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPCGAL_INTERNAL_POLYGONS_H
#define IMPCGAL_INTERNAL_POLYGONS_H

#include "../cgal_config.h"
#include <IMP/base_types.h>
#include <IMP/algebra/VectorD.h>
#include <IMP/algebra/BoundingBoxD.h>
#include <vector>


IMPCGAL_BEGIN_INTERNAL_NAMESPACE
IMPCGALEXPORT
std::pair<std::vector<std::vector<algebra::VectorD<3> > >, algebra::VectorD<3> >
get_convex_polygons(const std::vector<algebra::VectorD<3> > &poly);

IMPCGALEXPORT std::vector<algebra::VectorD<3> >
get_intersection(const algebra::VectorD<3> &normal,
                 double d,
                 const algebra::BoundingBoxD<3> &bb);

IMPCGAL_END_INTERNAL_NAMESPACE

#endif  /* IMPCGAL_INTERNAL_POLYGONS_H */
