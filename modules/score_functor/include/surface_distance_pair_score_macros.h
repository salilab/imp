/**
 *  \file IMP/score_functor/surface_distance_pair_score_macros.h
 *  \brief Various important macros for implementing decorators.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSCORE_FUNCTOR_SURFACE_DISTANCE_PAIR_SCORE_MACROS_H
#define IMPSCORE_FUNCTOR_SURFACE_DISTANCE_PAIR_SCORE_MACROS_H

#include "SurfaceDistancePairScore.h"

#if defined(SWIG) || defined(IMP_DOXYGEN)
/** Use this macro to define PairScores based on the
    IMP::score_functor::SurfaceDistancePairScore as it works around various
    complications caused by SWIG.

    To use it do something like
         IMP_FUNCTOR_SURFACE_DISTANCE_PAIR_SCORE(SurfaceDistancePairScore,
                                   SurfaceDistancePairScore,
                                   score_functor::UnaryFunctionEvaluate,
                            (UnaryFunction *uf, std::string name
                                         = "SurfaceDistancePairScore%1%"), (uf));
 */
#define IMP_FUNCTOR_SURFACE_DISTANCE_PAIR_SCORE(Name, PName, Functor, Args, PassArgs) \
  class Name : public IMP::PairScore {                                 \
    typedef IMP::score_functor::PName<Functor> P;          \
                                                                       \
   public:                                                             \
    Name Args;                                                         \
    double evaluate_index(Model *m,                            \
                          const ParticleIndexPair &pip,        \
                          DerivativeAccumulator *da) const;            \
    ModelObjectsTemp do_get_inputs(                            \
        Model *m, const ParticleIndexes &pis) const;   \
    IMP_OBJECT_METHODS(Name);                                          \
  }

#else
#define IMP_FUNCTOR_SURFACE_DISTANCE_PAIR_SCORE(Name, PName, Functor, Args, PassArgs) \
  class Name : public IMP::score_functor::PName<Functor> { \
    typedef IMP::score_functor::PName<Functor> P;          \
                                                                       \
   public:                                                             \
    Name Args : P(Functor PassArgs, name) {}                           \
  }
#endif

#endif /* IMPSCORE_FUNCTOR_SURFACE_DISTANCE_PAIR_SCORE_MACROS_H */
