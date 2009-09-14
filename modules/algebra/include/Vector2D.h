/**
 *  \file Vector2D.h   \brief Simple 2D vector class.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_VECTOR_2D_H
#define IMPALGEBRA_VECTOR_2D_H

#include <IMP/base_types.h>
#include <IMP/macros.h>
#include <IMP/exception.h>

#include <cmath>

#include "VectorD.h"

IMPALGEBRA_BEGIN_NAMESPACE

//! A location in 2D (x,y)
IMP_NO_SWIG(typedef VectorD<2> Vector2D;)

//! A collection of Vector2D
typedef std::vector<Vector2D> Vector2Ds;

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_VECTOR_2D_H */
