/**
 *  \file IMP/algebra/python_only.h
 *  \brief functionality only availble in python.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_PYTHON_ONLY_H
#define IMPALGEBRA_PYTHON_ONLY_H

#include <IMP/algebra/algebra_config.h>

IMPALGEBRA_BEGIN_NAMESPACE

/** \name Python Only
    The following functions are only availale in Python as the
    equivalent C++ functionality is provided via template
    functions or in other ways that don't directly map to
    python.
    @{
*/
#ifdef IMP_DOXYGEN
/*Sphere2D get_unit_sphere_2d();
Sphere3D get_unit_sphere_3d();
Sphere4D get_unit_sphere_4d();;
bool get_interiors_intersect(const Sphere2D&a, const Sphere2D &b);
bool get_interiors_intersect(const Sphere3D&a, const Sphere3D &b);
bool get_interiors_intersect(const Sphere4D&a, const Sphere4D &b);
double get_distance(const Sphere2D&a, const Sphere2D &b);
double get_distance(const Sphere3D&a, const Sphere3D &b);
double get_distance(const Sphere4D&a, const Sphere4D &b);
Vector2D get_random_vector_on(const Sphere2D &s);
Vector2D get_random_vector_on(const BoundingBox2D &s);
Vector3D get_random_vector_on(const Sphere3D &s);
Vector3D get_random_vector_on(const BoundingBox3D &s);
Vector4D get_random_vector_on(const Sphere4D &s);
Vector4D get_random_vector_on(const BoundingBox4D &s);
Vector2D get_random_vector_in(const Sphere2D &s);
Vector2D get_random_vector_in(const BoundingBox2D &s);
Vector3D get_random_vector_in(const Sphere3D &s);
Vector3D get_random_vector_in(const BoundingBox3D &s);
Vector4D get_random_vector_in(const Sphere4D &s);
Vector4D get_random_vector_in(const BoundingBox4D &s);
Vector2Ds get_uniform_surface_cover(const Sphere2D &s,
                                                 unsigned int n);
Vector3Ds get_uniform_surface_cover(const Sphere3D &s,
                                                 unsigned int n);
base::Vector<Vector4D > get_uniform_surface_cover(const Sphere4D &s,
                                                 unsigned int n);
BoundingBox2D get_bounding_box(const Sphere2D &s);
BoundingBox3D get_bounding_box(const Sphere3D &s);
BoundingBox4D get_bounding_box(const Sphere4D &s);

Vector2D get_basis_vector_2d();
Vector3D get_basis_vector_3d();
Vector4D get_basis_vector_4d();
Vector2D get_zero_vector_2d();
Vector3D get_zero_vector_3d();
Vector4D get_zero_vector_4d();
Vector2D get_ones_vector_2d();
Vector3D get_ones_vector_3d();
Vector4D get_ones_vector_4d();
BoundingBox2D get_unit_bounding_box_2d();
BoundingBox3D get_unit_bounding_box_3d();
BoundingBox4D get_unit_bounding_box_4d();
class SparseIntGrid3D;
class SparseUnboundedIntGrid3D;
class DenseDoubleGrid3D;
class DenseFloatGrid3D;
*/

/** Equivalent to
\code
IMP::algebra::get_transformation_aligning_first_to_second(a,b);
\endcode
 */
Transformation3D get_transformation_aligning_first_to_second(Vector3Ds a,
                                                             Vector3Ds b);

/** A sparse grid of integers over a bounded region of space.
 */
typedef Grid3D<int, SparseGridStorage3D<int, BoundedGridStorage3D> >
    SparseIntGrid3D;
/** A sparse grid of integers over all of space.
 */
typedef Grid3D<int, SparseGridStorage3D<int, UnboundedGridStorage3D> >
    SparseUnboundedIntGrid3D;
/** A grid of doubles over a region of space.
 */
typedef Grid3D<double, DenseGridStorage3D<double> > DenseDoubleGrid3D;
/** A grid of floats over a region of space.
 */
typedef Grid3D<float, DenseGridStorage3D<float> > DenseFloatGrid3D;
#endif
/** @} */

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_PYTHON_ONLY_H */
