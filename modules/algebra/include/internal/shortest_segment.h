/**
 *  \file cgal_predicates.h
 *  \brief predicates implemented using CGAL
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_SHORTEST_SEGMENT_H
#define IMPALGEBRA_INTERNAL_SHORTEST_SEGMENT_H

#include "../config.h"
#include "../Segment3D.h"
#include <IMP/base_types.h>

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

// defined in Segment3D.cpp

IMPALGEBRAEXPORT
Segment3D shortest_segment(const Segment3D &s,
                           const Vector3D &p);

IMPALGEBRAEXPORT
Segment3D shortest_segment(const Segment3D &sa,
                           const Segment3D &sb);
IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif  /* IMPALGEBRA_INTERNAL_SHORTEST_SEGMENT_H */
