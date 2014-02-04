/**
 *  \file IMP/algebra/connolly_surface.h     \brief Generate surface for a set
 *of
 *atoms
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_CONNOLLY_SURFACE_H
#define IMPALGEBRA_CONNOLLY_SURFACE_H

#include <IMP/algebra/algebra_config.h>
#include "Sphere3D.h"
#include <IMP/base/value_macros.h>
#include <IMP/base/showable_macros.h>
#include <IMP/base/Vector.h>

IMPALGEBRA_BEGIN_NAMESPACE

/** Represent a point on the connoly surface. */
class ConnollySurfacePoint {
  int atom[3];
  algebra::Vector3D surface_point;
  double area;
  algebra::Vector3D normal;

 public:
  ConnollySurfacePoint(int a0, int a1, int a2, algebra::Vector3D sp, float a,
                       algebra::Vector3D n)
      : surface_point(sp), area(a), normal(n) {
    atom[0] = a0;
    atom[1] = a1;
    atom[2] = a2;
  }
  int get_atom(unsigned int i) {
    IMP_USAGE_CHECK(i < 3, "Out of range");
    return atom[i];
  }
  const algebra::Vector3D& get_surface_point() const { return surface_point; }
  double get_area() const { return area; }
  const algebra::Vector3D& get_normal() const { return normal; }
  IMP_SHOWABLE_INLINE(ConnollySurfacePoint, out << surface_point);
};

IMP_VALUES(ConnollySurfacePoint, ConnollySurfacePoints);

/** The algorithm is taken from Connolly's original MS program, which is
    freely distributable and Copyright 1983, Michael Connolly.

    M.L. Connolly, "Solvent-accessible surfaces of proteins and nucleic acids",
    Science, 221, p709-713 (1983).

    M.L. Connolly, "Analytical molecular surface calculation",
    J. Appl. Cryst. 16, p548-558 (1983).

    See also get_uniform_surface_cover(const IMP::algebra::Sphere3Ds&, double)
    for a faster alternative.
 */
IMPALGEBRAEXPORT ConnollySurfacePoints
    get_connolly_surface(const algebra::Sphere3Ds& spheres, double density,
                         double probe_radius);

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_CONNOLLY_SURFACE_H */
