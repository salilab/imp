/**
 *  \file BoxSweepClosePairsFinder.h
 *  \brief Test all pairs of particles to find close pairs.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_BOX_SWEEP_CLOSE_PAIRS_FINDER_H
#define IMPCORE_BOX_SWEEP_CLOSE_PAIRS_FINDER_H

#include "ClosePairsFinder.h"
#include "core_macros.h"
#include "core_config.h"


#if defined(IMP_CORE_USE_IMP_CGAL) || defined(IMP_DOXYGEN)
IMPCORE_BEGIN_NAMESPACE

//! Find all nearby pairs by sweeping the bounding boxes
/** This method is much faster than the quadratic one when
    there are are large sets of points.

    \requires{class BoxSweepClosePairsFinder, CGAL}
    \see IMP::container::ClosePairsScoreState
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
