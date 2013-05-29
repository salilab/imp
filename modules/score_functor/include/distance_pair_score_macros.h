/**
 *  \file IMP/score_functor/distance_pair_score_macros.h
 *  \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSCORE_FUNCTOR_DISTANCE_PAIR_SCORE_MACROS_H
#define IMPSCORE_FUNCTOR_DISTANCE_PAIR_SCORE_MACROS_H

#include "DistancePairScore.h"

#if defined(SWIG) || defined(IMP_DOXYGEN)
#define IMP_FUNCTOR_DISTANCE_PAIR_SCORE(Name, Functor, Args, PassArgs) \
  class Name : public IMP::PairScore {                                 \
    typedef IMP::score_functor::DistancePairScore<Functor> P;          \
   public:                                                             \
    Name Args;                                                         \
    IMP_PAIR_SCORE(Name);                                              \
  }

#else
#define IMP_FUNCTOR_DISTANCE_PAIR_SCORE(Name, Functor, Args, PassArgs) \
  class Name : public IMP::score_functor::DistancePairScore<Functor> { \
    typedef IMP::score_functor::DistancePairScore<Functor> P;          \
   public:                                                             \
    Name Args : P(Functor PassArgs, name) {}                           \
  }
#endif

#endif /* IMPSCORE_FUNCTOR_DISTANCE_PAIR_SCORE_MACROS_H */
