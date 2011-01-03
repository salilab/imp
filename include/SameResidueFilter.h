/**
 *  \file example/SameResidueFilter.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_SAME_RESIDUE_FILTER_H
#define IMPMEMBRANE_SAME_RESIDUE_FILTER_H

#include "membrane_config.h"
#include <IMP/PairFilter.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//!
/**

*/
class IMPMEMBRANEEXPORT SameResidueFilter : public PairFilter
{
public:
  SameResidueFilter();
  IMP_PAIR_FILTER(SameResidueFilter);
};

IMP_OBJECTS(SameResidueFilter, SameResidueFilters);


IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_SAME_RESIDUE_FILTER_H */
