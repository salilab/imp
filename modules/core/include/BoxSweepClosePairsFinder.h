/**
 *  \file IMP/core/BoxSweepClosePairsFinder.h
 *  \brief Test all pairs of particles to find close pairs.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_BOX_SWEEP_CLOSE_PAIRS_FINDER_H
#define IMPCORE_BOX_SWEEP_CLOSE_PAIRS_FINDER_H

#include <IMP/core/core_config.h>
#include "ClosePairsFinder.h"
#include "close_pairs_finder_macros.h"

IMPCORE_BEGIN_NAMESPACE

#if defined(IMP_DOXYGEN) || defined(IMP_CORE_USE_IMP_CGAL)
//! Find all nearby pairs by sweeping the bounding boxes
/** This method is much faster than the quadratic one when
    there are are large sets of points.

    \requires{class BoxSweepClosePairsFinder, CGAL}
    \see IMP::container::ClosePairsScoreState
*/
class IMPCOREEXPORT BoxSweepClosePairsFinder : public ClosePairsFinder {
 public:
  BoxSweepClosePairsFinder();

  IMP_CLOSE_PAIRS_FINDER(BoxSweepClosePairsFinder);
};
#endif /* IMP_USE_CGAL */

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_BOX_SWEEP_CLOSE_PAIRS_FINDER_H */
