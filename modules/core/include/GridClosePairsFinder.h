/**
 *  \file GridClosePairsFinder.h
 *  \brief Use a hierarchy of grids to find close pairs.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_GRID_CLOSE_PAIRS_FINDER_H
#define IMPCORE_GRID_CLOSE_PAIRS_FINDER_H

#include "ClosePairsFinder.h"
#include "core_macros.h"
#include "core_config.h"

IMPCORE_BEGIN_NAMESPACE

//! Find all nearby pairs by testing all pairs
/**
   \see CloserPairsScoreState
 */
class IMPCOREEXPORT GridClosePairsFinder : public ClosePairsFinder
{
  algebra::BoundingBox3D bb_;
  unsigned int merged_;
 public:
  GridClosePairsFinder();

  enum {X=1, Y=2, Z=4} Boundary;
  /** Find close pairs with periodic boundary conditions. The
      bounding box is considered joined along X if
      merged_boundaries&X. Any point outside the bounding box
      in a periodic direction is considered to be at its
      image in the bounding box.
  */
  GridClosePairsFinder(const algebra::BoundingBox3D &bb,
                       unsigned int merged_boundaries= X|Y|Z);

  IMP_CLOSE_PAIRS_FINDER(GridClosePairsFinder);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_GRID_CLOSE_PAIRS_FINDER_H */
