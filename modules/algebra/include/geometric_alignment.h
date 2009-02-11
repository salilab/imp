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

//! Compute the rigid transform bringing the first point set to the second
/** The points are assumed to be corresponding (that is, from[0] is aligned
    to to[0] etc.). The alignment computed is that which minimized the
    sum of squared distances between corresponding points. That is
    \f$ \min_T \sum \left|T\left(f\left[i\right]\right)-t[i]\right|^2 \f$.

    If the point sets are only 1 or 2 dimensional, the alignment algorithm
    is unstable and not guaranteed to work. A warning is printed in this
    case.

    \throw ValueException if from.size() != to.size().
 */
IMPALGEBRAEXPORT Transformation3D
rigid_align_first_to_second(const std::vector<Vector3D> &from,
                            const std::vector<Vector3D> &to);




IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_GEOMETRIC_ALIGNMENT_H */
