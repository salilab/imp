/**
 *  \file Sphere3D.h   \brief Simple 3D sphere class.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPALGEBRA_SPHERE_3D_H
#define IMPALGEBRA_SPHERE_3D_H

#include "SphereD.h"

IMPALGEBRA_BEGIN_NAMESPACE



//! Return a sphere containing the listed spheres
/** \relatesalso SphereD<3>
    \note This method produces tighter bounding spheres if CGAL
    is used.
    \ingroup CGAL
 */
IMPALGEBRAEXPORT SphereD<3>
get_enclosing_sphere(const std::vector<SphereD<3> > &ss);

//! Return a sphere containing the listed spheres
/** \relatesalso SphereD<3>
    \relatesalso VectorD<3>
    \note This method produces tighter bounding spheres if CGAL
    is used.
    \ingroup CGAL
 */
IMPALGEBRAEXPORT SphereD<3>
get_enclosing_sphere(const std::vector<VectorD<3> > &ss);


//! Return the radius of a sphere with a given volume
/** \relatesalso SphereD<3>
 */
inline double get_ball_radius_from_volume_3d(double volume) {
  return std::pow((.75/PI)*volume, .3333);
}


#ifdef IMP_ALGEBRA_USE_IMP_CGAL
/** Return the surface area and volume of the union of the balls
    bounded by the spheres. This method requires CGAL to work.
*/
IMPALGEBRAEXPORT
FloatPair
get_surface_area_and_volume(const std::vector<algebra::SphereD<3> > &ss);
#endif

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_SPHERE_3D_H */
