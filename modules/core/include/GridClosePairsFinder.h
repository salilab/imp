/**
 *  \file GridClosePairsFinder.h
 *  \brief Use a hierarchy of grids to find close pairs.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
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
 public:
  GridClosePairsFinder();

  IMP_CLOSE_PAIRS_FINDER(GridClosePairsFinder);
  ParticleIndexPairs get_close_pairs(Model *m,
                                     const ParticleIndexes &pc) const;
  ParticleIndexPairs get_close_pairs(Model *m,
                                     const ParticleIndexes &pca,
                                     const ParticleIndexes &pcb) const;

};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_GRID_CLOSE_PAIRS_FINDER_H */
