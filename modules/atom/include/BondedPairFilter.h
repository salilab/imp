/**
 *  \file IMP/atom/BondedPairFilter.h
 *  \brief A fake container for bonds
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_BONDED_PAIR_FILTER_H
#define IMPATOM_BONDED_PAIR_FILTER_H

#include <IMP/atom/atom_config.h>
#include "bond_decorators.h"
#include <IMP/PairPredicate.h>
#include <IMP/pair_macros.h>
IMPATOM_BEGIN_NAMESPACE

//! A filter for bonds.
/** This is to be used with a core::ClosePairsScoreState to exclude all
    bonded pairs.
    \ingroup bond
    \see Bonded
 */
class IMPATOMEXPORT BondedPairFilter : public PairPredicate {
 public:
  //! no arguments
  BondedPairFilter();

  virtual int get_value_index(kernel::Model *m,
                              const kernel::ParticleIndexPair &p) const
      IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_PAIR_PREDICATE_METHODS(BondedPairFilter);
  IMP_OBJECT_METHODS(BondedPairFilter);
  ;
};

IMP_OBJECTS(BondedPairFilter, BondedPairFilters);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_BONDED_PAIR_FILTER_H */
