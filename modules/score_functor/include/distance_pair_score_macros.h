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
/** Use this macro to define PairScores based on the
    IMP::score_functor::DistancePairScore as it works around various
    complications caused by SWIG.

    To use it do something like
         IMP_FUNCTOR_DISTANCE_PAIR_SCORE(DistancePairScore,
                                   score_functor::UnaryFunctionEvaluate,
                            (UnaryFunction *uf, std::string name
                                         = "DistancePairScore%1%"), (uf));
 */
#define IMP_FUNCTOR_DISTANCE_PAIR_SCORE(Name, Functor, Args, PassArgs) \
  class Name : public IMP::PairScore {                                 \
    typedef IMP::score_functor::DistancePairScore<Functor> P;          \
                                                                       \
   public:                                                             \
    Name Args;                                                         \
    double evaluate_index(kernel::Model *m,                            \
                          const kernel::ParticleIndexPair &pip,        \
                          DerivativeAccumulator *da) const;            \
    kernel::ModelObjectsTemp do_get_inputs(                            \
        kernel::Model *m, const kernel::ParticleIndexes &pis) const;   \
    IMP_OBJECT_METHODS(Name);                                          \
  }

#else
#define IMP_FUNCTOR_DISTANCE_PAIR_SCORE(Name, Functor, Args, PassArgs) \
  class Name : public IMP::score_functor::DistancePairScore<Functor> { \
    typedef IMP::score_functor::DistancePairScore<Functor> P;          \
                                                                       \
   public:                                                             \
    Name Args : P(Functor PassArgs, name) {}                           \
  }
#endif

#endif /* IMPSCORE_FUNCTOR_DISTANCE_PAIR_SCORE_MACROS_H */
