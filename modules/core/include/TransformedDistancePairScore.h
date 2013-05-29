/**
 *  \file IMP/core/TransformedDistancePairScore.h
 *  \brief A score on the distance between a pair of particles
 *  after transforming one.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_TRANSFORMED_DISTANCE_PAIR_SCORE_H
#define IMPCORE_TRANSFORMED_DISTANCE_PAIR_SCORE_H

#include <IMP/core/core_config.h>
#include <IMP/generic.h>
#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>
#include <IMP/pair_macros.h>

#include <IMP/algebra/Transformation3D.h>

IMPCORE_BEGIN_NAMESPACE

/** \brief  Apply a function to the distance between two particles
    after transforming the first

    Apply a transform to the second particle and then apply the unary
    function to the distance between the transformed particle and the
    second. This can be used to implement symmetry restraints.
 */
class IMPCOREEXPORT TransformedDistancePairScore : public PairScore {
  IMP::OwnerPointer<UnaryFunction> f_;
  algebra::Transformation3D t_;
  algebra::Rotation3D ri_;

 public:
  TransformedDistancePairScore(UnaryFunction *f,
                               const algebra::Transformation3D &transformation);

  /** Set the transformation object.*/
  void set_transformation(const algebra::Transformation3D &rot);
  IMP_SIMPLE_PAIR_SCORE(TransformedDistancePairScore);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_TRANSFORMED_DISTANCE_PAIR_SCORE_H */
