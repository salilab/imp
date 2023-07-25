/**
 *  \file cgal/internal/sphere_cover.h
 *  \brief Generate points that cover a sphere.
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
*/

#ifndef IMPCGAL_INTERNAL_SPHERE_COVER_H
#define IMPCGAL_INTERNAL_SPHERE_COVER_H

#include <IMP/cgal/cgal_config.h>
#include <IMP/types.h>
#include <IMP/algebra/VectorD.h>
#include <vector>

IMPCGAL_BEGIN_INTERNAL_NAMESPACE
IMPCGALEXPORT void refine_unit_sphere_cover_4d(
    Vector<algebra::VectorD<4> > &pts, bool ALL);

IMPCGAL_END_INTERNAL_NAMESPACE

#endif /* IMPCGAL_INTERNAL_SPHERE_COVER_H */
