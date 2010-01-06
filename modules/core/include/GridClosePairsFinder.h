/**
 *  \file GridClosePairsFinder.h
 *  \brief Use a hierarchy of grids to find close pairs.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_GRID_CLOSE_PAIRS_FINDER_H
#define IMPCORE_GRID_CLOSE_PAIRS_FINDER_H

#include "ClosePairsFinder.h"
#include "macros.h"
#include "config.h"

IMPCORE_BEGIN_NAMESPACE

//! Find all nearby pairs by testing all pairs
/**
   \see CloserPairsScoreState
 */
class IMPCOREEXPORT GridClosePairsFinder : public ClosePairsFinder
{
 public:
  GridClosePairsFinder();

  IMP_CLOSE_PAIRS_FINDER(GridClosePairsFinder, get_module_version_info());
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_GRID_CLOSE_PAIRS_FINDER_H */
