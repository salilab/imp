/**
 *  \file bounding_sphere.h   \brief Simple 3D sphere class.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPCGAL_INTERNAL_BOUNDING_SPHERE_H
#define IMPCGAL_INTERNAL_BOUNDING_SPHERE_H
#include "../cgal_config.h"
#include <IMP/algebra/SphereD.h>
#include <vector>
IMPCGAL_BEGIN_INTERNAL_NAMESPACE

IMPCGALEXPORT algebra::SphereD<3>
get_enclosing_sphere(const std::vector<algebra::SphereD<3> > &ss);



IMPCGAL_END_INTERNAL_NAMESPACE

#endif /* IMPCGAL_INTERNAL_BOUNDING_SPHERE_H */
