/**
 *  \file geometric_alignment.h
 *  \brief align sets of points.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPALGEBRA_GEOMETRIC_ALIGNMENT_H
#define IMPALGEBRA_GEOMETRIC_ALIGNMENT_H

#include "config.h"

#include "Vector3D.h"
#include "Rotation3D.h"
#include "Transformation3D.h"
#include <vector>


IMPALGEBRA_BEGIN_NAMESPACE

IMPALGEBRAEXPORT Transformation3D
rigid_align_first_to_second(const std::vector<Vector3D> &from,
                            const std::vector<Vector3D> &to);




IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_GEOMETRIC_ALIGNMENT_H */
