/**
 *  \file NearestNeighborsClosePairsFinder.h
 *  \brief Test all pairs of particles to find close pairs.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_NEAREST_NEIGHBORS_CLOSE_PAIRS_FINDER_H
#define IMPCORE_NEAREST_NEIGHBORS_CLOSE_PAIRS_FINDER_H

#include "ClosePairsFinder.h"
#include "core_macros.h"
#include "core_config.h"


IMPCORE_BEGIN_NAMESPACE

//! Find all nearby pairs using the algebra::NearestNeighbor code
/** \see IMP::container::ClosePairsScoreState
    \ingroup cgal
*/
class IMPCOREEXPORT NearestNeighborsClosePairsFinder : public ClosePairsFinder
{
 public:
  NearestNeighborsClosePairsFinder();

  IMP_CLOSE_PAIRS_FINDER(NearestNeighborsClosePairsFinder);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_NEAREST_NEIGHBORS_CLOSE_PAIRS_FINDER_H */
