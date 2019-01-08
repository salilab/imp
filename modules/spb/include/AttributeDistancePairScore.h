/**
 *  \file IMP/spb/AttributeDistancePairScore.h
 *  \brief A score based on the unmodified value of an attribute.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 */

#ifndef IMPSPB_ATTRIBUTE_DISTANCE_PAIR_SCORE_H
#define IMPSPB_ATTRIBUTE_DISTANCE_PAIR_SCORE_H

#include <IMP/PairScore.h>
#include <IMP/Pointer.h>
#include <IMP/UnaryFunction.h>
#include <IMP/pair_macros.h>
#include "spb_config.h"

IMPSPB_BEGIN_NAMESPACE

//! Apply a function to an attribute.
/** This Score scores a particle by passing an attribute value directly
    to a UnaryFunction.
 */
class IMPSPBEXPORT AttributeDistancePairScore : public PairScore {
  // IMP::OwnerPointer<UnaryFunction> f_; //replaced deprecated version
  IMP::PointerMember<UnaryFunction> f_;
  FloatKey k_;

 public:
  //! Apply function f to attribute k
  AttributeDistancePairScore(UnaryFunction *f, FloatKey k);

  virtual double evaluate_index(IMP::Model *m, const IMP::ParticleIndexPair &p,
                                DerivativeAccumulator *da) const IMP_OVERRIDE;

  virtual IMP::ModelObjectsTemp do_get_inputs(
      IMP::Model *m, const IMP::ParticleIndexes &pis) const IMP_OVERRIDE;

  // void show(std::ostream &out) const ;

  IMP_PAIR_SCORE_METHODS(AttributeDistancePairScore);
  IMP_OBJECT_METHODS(AttributeDistancePairScore);
  IMP_SHOWABLE(AttributeDistancePairScore);

  /** deprecated code
    IMP_SIMPLE_PAIR_SCORE(AttributeDistancePairScore);
   */
};

IMP_OBJECTS(AttributeDistancePairScore, AttributeDistancePairScores);

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_ATTRIBUTE_DISTANCE_PAIR_SCORE_H */
