/**
 *  \file IMP/core/QuadraticClosePairsFinder.h
 *  \brief Test all pairs of particles to find close pairs.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_QUADRATIC_CLOSE_PAIRS_FINDER_H
#define IMPCORE_QUADRATIC_CLOSE_PAIRS_FINDER_H

#include "ClosePairsFinder.h"
#include "close_pairs_finder_macros.h"
#include <IMP/core/core_config.h>

IMPCORE_BEGIN_NAMESPACE

//! Find all nearby pairs by testing all pairs
/** \see ClosePairsScoreState
 */
class IMPCOREEXPORT QuadraticClosePairsFinder : public ClosePairsFinder {
 public:
  QuadraticClosePairsFinder();
  bool get_are_close(Particle *a, Particle *b) const;
  IMP_CLOSE_PAIRS_FINDER(QuadraticClosePairsFinder);

};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_QUADRATIC_CLOSE_PAIRS_FINDER_H */
