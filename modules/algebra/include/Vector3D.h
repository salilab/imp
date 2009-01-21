/**
 *  \file Vector3D.h   \brief Simple 3D vector class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_VECTOR_3D_H
#define IMPALGEBRA_VECTOR_3D_H

#include <IMP/base_types.h>
#include <IMP/macros.h>
#include <IMP/exception.h>

#include <cmath>

#include "VectorD.h"

IMPALGEBRA_BEGIN_NAMESPACE

//! A location in 3D
typedef VectorD<3> Vector3D;

//! A collection of Vector3D
typedef std::vector<Vector3D> Vector3Ds;

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_VECTOR_3D_H */
