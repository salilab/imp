/**
 *  \file atom/BondedPairFilter.h
 *  \brief A fake container for bonds
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_BONDED_PAIR_FILTER_H
#define IMPATOM_BONDED_PAIR_FILTER_H

#include "atom_config.h"
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
class IMPATOMEXPORT BondedPairFilter : public PairPredicate
{
public:
  //! no arguments
  BondedPairFilter();

  IMP_INDEX_PAIR_PREDICATE(BondedPairFilter, {
      Particle *pa= m->get_particle(pi[0]);
      Particle *pb= m->get_particle(pi[1]);
      if (!Bonded::particle_is_instance(pa)
          || ! Bonded::particle_is_instance(pb)) {
        return false;
      }
      Bonded ba(m, pi[0]);
      Bonded bb(m, pi[1]);
      Bond bd=get_bond(ba, bb);
      return bd != Bond();
    });
};

IMP_OBJECTS(BondedPairFilter,BondedPairFilters);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_BONDED_PAIR_FILTER_H */
