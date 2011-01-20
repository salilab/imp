/**
 *  \file atom/SameResiduePairFilter.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_SAME_RESIDUE_PAIR_FILTER_H
#define IMPATOM_SAME_RESIDUE_PAIR_FILTER_H

#include "atom_config.h"
#include <IMP/PairFilter.h>

IMPATOM_BEGIN_NAMESPACE

//!
/** Filter atoms that belong to the same residue. The is designed
    for use with the DopePairScore.
*/
class IMPATOMEXPORT SameResiduePairFilter : public PairFilter
{
public:
  SameResiduePairFilter();
  IMP_PAIR_FILTER(SameResiduePairFilter);
};


IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_SAME_RESIDUE_PAIR_FILTER_H */
