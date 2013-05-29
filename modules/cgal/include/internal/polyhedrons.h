/**
 *  \file cgal/internal/polyhedrons.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPCGAL_INTERNAL_POLYHEDRONS_H
#define IMPCGAL_INTERNAL_POLYHEDRONS_H

#include <IMP/cgal/cgal_config.h>
#include <IMP/base/types.h>
#include <IMP/algebra/VectorD.h>
#include <IMP/algebra/SphereD.h>
#include <IMP/algebra/Plane3D.h>
#include <IMP/algebra/standard_grids.h>
#include <vector>

IMPCGAL_BEGIN_INTERNAL_NAMESPACE
/** compute the polygon as the subtraction of the intersection of
    the various planes
 */
IMPCGALEXPORT base::Vector<algebra::Vector3Ds> get_polyhedron_facets(
    const algebra::BoundingBoxD<3> &bb,
    const base::Vector<algebra::Plane3D> &outer,
    const base::Vector<algebra::Plane3D> &hole);

IMPCGALEXPORT base::Vector<algebra::Vector3Ds> get_polyhedron_facets(
    const algebra::BoundingBoxD<3> &bb,
    const base::Vector<algebra::Plane3D> &outer);

IMPCGALEXPORT std::pair<algebra::Vector3Ds, Ints> get_polyhedron_indexed_facets(
    const algebra::BoundingBoxD<3> &bb,
    const base::Vector<algebra::Plane3D> &outer,
    const base::Vector<algebra::Plane3D> &hole);

IMPCGALEXPORT std::pair<algebra::Vector3Ds, Ints> get_skin_surface(
    const algebra::Sphere3Ds &ss);

IMPCGALEXPORT std::pair<algebra::Vector3Ds, Ints> get_iso_surface(
    const algebra::GridD<3, algebra::DenseGridStorageD<3, double>, double> &
        grid,
    double iso_level);

IMPCGALEXPORT std::pair<algebra::Vector3Ds, Ints> get_iso_surface(
    const algebra::GridD<3, algebra::DenseGridStorageD<3, float>, float> &grid,
    double iso_level);

IMPCGAL_END_INTERNAL_NAMESPACE

#endif /* IMPCGAL_INTERNAL_POLYHEDRONS_H */
