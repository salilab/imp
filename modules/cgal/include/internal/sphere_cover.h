/**
 *  \file cgal/internal/polyhedrons.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPCGAL_INTERNAL_SPHERE_COVER_H
#define IMPCGAL_INTERNAL_SPHERE_COVER_H

#include <IMP/cgal/cgal_config.h>
#include <IMP/base/types.h>
#include <IMP/algebra/VectorD.h>
#include <vector>

IMPCGAL_BEGIN_INTERNAL_NAMESPACE
IMPCGALEXPORT void refine_unit_sphere_cover_4d(
    base::Vector<algebra::VectorD<4> > &pts, bool ALL);

IMPCGAL_END_INTERNAL_NAMESPACE

#endif /* IMPCGAL_INTERNAL_SPHERE_COVER_H */
