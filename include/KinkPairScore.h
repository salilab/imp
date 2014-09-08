/**
 *  \file KinkPairScore.h
 *  \brief A Score on the crossing angle between two rigid bodies
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_KINK_PAIR_SCORE_H
#define IMPMEMBRANE_KINK_PAIR_SCORE_H

#include "membrane_config.h"

#include <IMP/PairScore.h>
#include <IMP/pair_macros.h>
#include <IMP/base/Pointer.h>
#include <IMP/UnaryFunction.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//! Score on the crossing angles between two helices
class IMPMEMBRANEEXPORT KinkPairScore : public PairScore
{
  IMP::base::PointerMember<UnaryFunction> f_;
public:
  KinkPairScore(UnaryFunction *f);

 virtual double evaluate_index(kernel::Model *m,
   const kernel::ParticleIndexPair &p,
    DerivativeAccumulator *da) const IMP_OVERRIDE;

 virtual kernel::ModelObjectsTemp do_get_inputs(kernel::Model *m,
   const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;

 // void show(std::ostream &out) const ;

  IMP_PAIR_SCORE_METHODS(KinkPairScore);
  IMP_OBJECT_METHODS(KinkPairScore);
  IMP_SHOWABLE(KinkPairScore);

};

IMP_OBJECTS(KinkPairScore, KinkPairScores);


IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_KINK_PAIR_SCORE_H */
