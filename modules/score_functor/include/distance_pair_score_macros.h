/**
 *  \file IMP/score_functor/distance_pair_score_macros.h
 *  \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSCORE_FUNCTOR_DISTANCE_PAIR_SCORE_MACROS_H
#define IMPSCORE_FUNCTOR_DISTANCE_PAIR_SCORE_MACROS_H

#include "DistancePairScore.h"
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

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
    Name() {}                                                          \
    double evaluate_index(Model *m,                            \
                          const ParticleIndexPair &pip,        \
                          DerivativeAccumulator *da) const;            \
    ModelObjectsTemp do_get_inputs(                            \
        Model *m, const ParticleIndexes &pis) const;   \
    IMP_OBJECT_METHODS(Name);                                          \
  }

#else
#define IMP_FUNCTOR_DISTANCE_PAIR_SCORE(Name, Functor, Args, PassArgs) \
  class Name : public IMP::score_functor::DistancePairScore<Functor> { \
    typedef IMP::score_functor::DistancePairScore<Functor> P;          \
    friend class cereal::access;                                       \
    template<class Archive> void serialize(Archive &ar) {              \
      ar(cereal::base_class<                                           \
          IMP::score_functor::DistancePairScore<Functor> >(this));     \
    }                                                                  \
    IMP_OBJECT_SERIALIZE_DECL(Name);                                   \
                                                                       \
   public:                                                             \
    Name Args : P(Functor PassArgs, name) {}                           \
    Name() {}                                                          \
  }
#endif

#endif /* IMPSCORE_FUNCTOR_DISTANCE_PAIR_SCORE_MACROS_H */
