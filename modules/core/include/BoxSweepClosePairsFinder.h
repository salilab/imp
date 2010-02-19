/**
 *  \file BoxSweepClosePairsFinder.h
 *  \brief Test all pairs of particles to find close pairs.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_BOX_SWEEP_CLOSE_PAIRS_FINDER_H
#define IMPCORE_BOX_SWEEP_CLOSE_PAIRS_FINDER_H

#include "ClosePairsFinder.h"
#include "macros.h"
#include "config.h"


#ifdef IMP_USE_CGAL
IMPCORE_BEGIN_NAMESPACE

//! Find all nearby pairs by sweeping the bounding boxes
/** This method is much faster than the quadratic one when
    there are are large sets of points.

    \note This method requires \ref cgal "CGAL" to work.
    \see IMP::container::ClosePairsScoreState
    \ingroup cgal
*/
class IMPCOREEXPORT BoxSweepClosePairsFinder : public ClosePairsFinder
{
 public:
  BoxSweepClosePairsFinder();

  IMP_CLOSE_PAIRS_FINDER(BoxSweepClosePairsFinder);
};

IMPCORE_END_NAMESPACE

#endif /* IMP_USE_CGAL */

#endif  /* IMPCORE_BOX_SWEEP_CLOSE_PAIRS_FINDER_H */
