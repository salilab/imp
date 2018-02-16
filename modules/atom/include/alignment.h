/**
 *  \file IMP/atom/alignment.h
 *  \brief Alignment of structures
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_ALIGNMENT_H
#define IMPATOM_ALIGNMENT_H

#include <IMP/atom/atom_config.h>
#include <IMP/core/XYZ.h>
#include "Selection.h"
#include <IMP/algebra.h>

IMPATOM_BEGIN_NAMESPACE

//! Get the transformation to align two selections
IMPATOMEXPORT algebra::Transformation3D
    get_transformation_aligning_first_to_second(const Selection &s1,
                                                const Selection &s2);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_ALIGNMENT_H */
