/**
 *  \file IMP/spb/KinkPairScore.h
 *  \brief A Score on the crossing angle between two rigid bodies
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 */

#ifndef IMPSPB_KINK_PAIR_SCORE_H
#define IMPSPB_KINK_PAIR_SCORE_H

#include "spb_config.h"

#include <IMP/Pointer.h>
#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/pair_macros.h>

IMPSPB_BEGIN_NAMESPACE

//! Score on the crossing angles between two helices
class IMPSPBEXPORT KinkPairScore : public PairScore {
  IMP::PointerMember<UnaryFunction> f_;

 public:
  KinkPairScore(UnaryFunction *f);

  virtual double evaluate_index(IMP::Model *m, const IMP::ParticleIndexPair &p,
                                DerivativeAccumulator *da) const IMP_OVERRIDE;

  virtual IMP::ModelObjectsTemp do_get_inputs(
      IMP::Model *m, const IMP::ParticleIndexes &pis) const IMP_OVERRIDE;

  // void show(std::ostream &out) const ;

  IMP_PAIR_SCORE_METHODS(KinkPairScore);
  IMP_OBJECT_METHODS(KinkPairScore);
  IMP_SHOWABLE(KinkPairScore);
};

IMP_OBJECTS(KinkPairScore, KinkPairScores);

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_KINK_PAIR_SCORE_H */
