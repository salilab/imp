/**
 *  \file bounding_sphere.h   \brief Simple 3D sphere class.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCGAL_INTERNAL_BOUNDING_SPHERE_H
#define IMPCGAL_INTERNAL_BOUNDING_SPHERE_H
#include <IMP/cgal/cgal_config.h>
#include <IMP/algebra/SphereD.h>
#include <vector>
IMPCGAL_BEGIN_INTERNAL_NAMESPACE

IMPCGALEXPORT algebra::Sphere3D get_enclosing_sphere(
    const algebra::Sphere3Ds &ss);

IMPCGAL_END_INTERNAL_NAMESPACE

#endif /* IMPCGAL_INTERNAL_BOUNDING_SPHERE_H */
