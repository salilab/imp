/**
 *  \file  Ellipsoid3D.cpp
 *  \brief simple implementation of ellipsoids in 3D
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP/algebra/Ellipsoid3D.h>

IMPALGEBRA_BEGIN_NAMESPACE
Ellipsoid3D::Ellipsoid3D(const Vector3D& center, double r0, double r1,
                         double r2, const Rotation3D& rot)
    : rf_(Transformation3D(rot, center)), radii_(r0, r1, r2) {}

IMPALGEBRA_END_NAMESPACE
