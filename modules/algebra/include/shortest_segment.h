/**
 *  \file IMP/algebra/shortest_segment.h
 *  \brief predicates implemented using CGAL
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_SHORTEST_SEGMENT_H
#define IMPALGEBRA_SHORTEST_SEGMENT_H

#include <IMP/algebra/algebra_config.h>
#include "Segment3D.h"
#include <IMP/base/types.h>

IMPALGEBRA_BEGIN_NAMESPACE

/** \name Shortest segments

    These methods return the shortest segment connecting two geometric
    objects. Such segments can be used to give the direction of the
    derivative of the distance between the two objects. The 0 point on
    the segment is in the first passed object and the 1 point is in
    the second.
*/
/** \see Segment3D
    \see Vector3D
*/
IMPALGEBRAEXPORT Segment3D
    get_shortest_segment(const Segment3D &s, const Vector3D &p);

/** \see Segment3D
 */
IMPALGEBRAEXPORT Segment3D
    get_shortest_segment(const Segment3D &sa, const Segment3D &sb);

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_SHORTEST_SEGMENT_H */
