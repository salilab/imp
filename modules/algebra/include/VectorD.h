/**
 *  \file IMP/algebra/VectorD.h   \brief Simple D vector class.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 */

#ifndef IMPALGEBRA_VECTOR_D_H
#define IMPALGEBRA_VECTOR_D_H

#include <IMP/algebra/algebra_config.h>
#include <IMP/VectorD.h>

IMPALGEBRA_BEGIN_NAMESPACE

// VectorD is actually implemented in the kernel (since it is used by
// IMP::Model) so just add aliases here
using IMP::VectorD;
using IMP::get_squared_distance;
using IMP::get_distance;
using IMP::get_basis_vector_d;
using IMP::get_basis_vector_kd;
using IMP::get_zero_vector_d;
using IMP::get_zero_vector_kd;
using IMP::get_ones_vector_d;
using IMP::get_ones_vector_kd;
using IMP::get_l2_norm;
using IMP::get_l1_norm;
using IMP::get_linf_norm;
#ifndef SWIG
using IMP::get_vector_geometry;
using IMP::set_vector_geometry;
#endif
using IMP::get_elementwise_product;
using IMP::Vector1D;
using IMP::Vector1Ds;
using IMP::Vector2D;
using IMP::Vector2Ds;
using IMP::Vector3D;
using IMP::Vector3Ds;
using IMP::Vector4D;
using IMP::Vector4Ds;
using IMP::Vector5D;
using IMP::Vector5Ds;
using IMP::Vector6D;
using IMP::Vector6Ds;
using IMP::VectorKD;
using IMP::VectorKDs;
using IMP::commas_io;

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_VECTOR_D_H */
