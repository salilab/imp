/**
 *  \file core/utility.cpp
 *  \brief Functions to perform simple functions on a set of particles
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#include "IMP/core/utility.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/XYZ.h>
IMPCORE_BEGIN_NAMESPACE

algebra::VectorD<3> get_centroid(const XYZsTemp &ps) {
 algebra::VectorD<3> cen(0.0,0.0,0.0);
 for (XYZsTemp::const_iterator it = ps.begin(); it != ps.end();it++) {
   cen = cen + it->get_coordinates();
 }
 return cen/ps.size();
}

IMPCORE_END_NAMESPACE
