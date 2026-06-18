/**
 *  \file IMP/algebra/Vector3D.h   \brief Simple 3D vector class.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 */

#ifndef IMPALGEBRA_VECTOR_3D_H
#define IMPALGEBRA_VECTOR_3D_H

#include <IMP/algebra/VectorD.h>
#include <IMP/Vector3D.h>

IMPALGEBRA_BEGIN_NAMESPACE

// Vector3D is actually implemented in the kernel (since it is used by
// IMP::Model) so just add aliases here
using IMP::get_vector_product;
using IMP::get_orthogonal_vector;
using IMP::get_centroid;
using IMP::get_radius_of_gyration;

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_VECTOR_3D_H */
