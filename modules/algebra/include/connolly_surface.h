/**
 *  \file algebra/connolly_surface.h     \brief Generate surface for a set of
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
struct ConnollySurfacePoint {
  ConnollySurfacePoint(int a0, int a1, int a2, algebra::Vector3D sp, float a,
                       algebra::Vector3D n)
      : atom_0(a0),
        atom_1(a1),
        atom_2(a2),
        surface_point(sp),
        area(a),
        normal(n) {}
  int atom_0;
  int atom_1;
  int atom_2;
  algebra::Vector3D surface_point;
  float area;
  algebra::Vector3D normal;
  IMP_SHOWABLE_INLINE(ConnollySurfacePoint, out << surface_point);
};

IMP_VALUES(ConnollySurfacePoint, ConnollySurfacePoints);

/** The algorithm is taken from Connolly's original MS program, which is
    freely distributable and Copyright 1983, Michael Connolly.

    M.L. Connolly, "Solvent-accessible surfaces of proteins and nucleic acids",
    Science, 221, p709-713 (1983).

    M.L. Connolly, "Analytical molecular surface calculation",
    J. Appl. Cryst. 16, p548-558 (1983).
 */
ConnollySurfacePoints IMPALGEBRAEXPORT
    get_connolly_surface(const algebra::Sphere3Ds& spheres, float density,
                         float probe_radius);

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_CONNOLLY_SURFACE_H */
