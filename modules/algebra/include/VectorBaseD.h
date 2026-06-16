/**
 *  \file IMP/algebra/VectorBaseD.h   \brief Simple D vector class.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_VECTOR_BASE_D_H
#define IMPALGEBRA_VECTOR_BASE_D_H

#include <IMP/algebra/algebra_config.h>
#include <IMP/internal/VectorBaseD.h>

IMPALGEBRA_BEGIN_NAMESPACE

IMPALGEBRA_DEPRECATED_HEADER(2.25, "Use IMP/internal/VectorBaseD.h instead")

using IMP::internal::VectorBaseD;
using IMP::internal::get_unit_vector;
using IMP::internal::get_magnitude_and_normalize_in_place;

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_VECTOR_BASE_D_H */
