/**
 *  \file AttributeDistancePairScore.h
 *  \brief A score based on the unmodified value of an attribute.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_ATTRIBUTE_DISTANCE_PAIR_SCORE_H
#define IMPMEMBRANE_ATTRIBUTE_DISTANCE_PAIR_SCORE_H

#include "membrane_config.h"
#include <IMP/PairScore.h>
#include <IMP/base/Pointer.h>
#include <IMP/UnaryFunction.h>
#include <IMP/pair_macros.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//! Apply a function to an attribute.
/** This Score scores a particle by passing an attribute value directly
    to a UnaryFunction.
 */
class IMPMEMBRANEEXPORT AttributeDistancePairScore : public PairScore
{
  //IMP::OwnerPointer<UnaryFunction> f_; //replaced deprecated version
  IMP::base::PointerMember<UnaryFunction> f_;
  FloatKey k_;
public:
  //! Apply function f to attribute k
  AttributeDistancePairScore(UnaryFunction *f, FloatKey k);

   virtual double evaluate_index(kernel::Model *m,
   const kernel::ParticleIndexPair &p,
    DerivativeAccumulator *da) const IMP_OVERRIDE;

   virtual kernel::ModelObjectsTemp do_get_inputs(kernel::Model *m,
   const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;

   void show(std::ostream &out) const ;


   IMP_PAIR_SCORE_METHODS(AttributeDistancePairScore);
   IMP_OBJECT_METHODS(AttributeDistancePairScore);

/** deprecated code
  IMP_SIMPLE_PAIR_SCORE(AttributeDistancePairScore);
 */
};


IMP_OBJECTS(AttributeDistancePairScore, AttributeDistancePairScores);

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_ATTRIBUTE_DISTANCE_PAIR_SCORE_H */
