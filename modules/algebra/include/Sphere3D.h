/**
 *  \file IMP/algebra/Sphere3D.h   \brief Simple 3D sphere class.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_SPHERE_3D_H
#define IMPALGEBRA_SPHERE_3D_H

#include "SphereD.h"

IMPALGEBRA_BEGIN_NAMESPACE

//! Return a sphere containing the listed spheres
/** See Sphere3D
    \note This method produces tighter bounding spheres if CGAL
    is used.
    \ingroup CGAL
 */
IMPALGEBRAEXPORT Sphere3D get_enclosing_sphere(const Sphere3Ds &ss);

//! Return a sphere containing the listed spheres
/** See Sphere3D
    See Vector3D
    \note This method produces tighter bounding spheres if CGAL
    is used.
    \ingroup CGAL
 */
IMPALGEBRAEXPORT Sphere3D get_enclosing_sphere(const Vector3Ds &ss);

//! Return the radius of a sphere with a given volume
/** See Sphere3D
 */
inline double get_ball_radius_from_volume_3d(double volume) {
  IMP_USAGE_CHECK(volume >= 0, "Volume can't be negative");
  return std::pow((.75 / PI) * volume, .3333);
}

#ifdef IMP_ALGEBRA_USE_IMP_CGAL
/** Return the surface area and volume of the union of the balls
    bounded by the spheres. This method requires CGAL to work.
*/
IMPALGEBRAEXPORT FloatPair get_surface_area_and_volume(
    const algebra::Sphere3Ds &ss);
#endif

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_SPHERE_3D_H */
