/**
 *  \file IMP/core/AngleTripletScore.h
 *  \brief A Score on the angle between three particles.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_ANGLE_TRIPLET_SCORE_H
#define IMPCORE_ANGLE_TRIPLET_SCORE_H

#include <IMP/core/core_config.h>
#include <IMP/generic.h>
#include <IMP/TripletScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>
#include <IMP/triplet_macros.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPCORE_BEGIN_NAMESPACE

//! Apply a function to the angle between three particles.
/** */
class IMPCOREEXPORT AngleTripletScore : public TripletScore {
  IMP::PointerMember<UnaryFunction> f_;

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<TripletScore>(this), f_);
  }

 public:
  //! Score the angle (in radians) using f
  AngleTripletScore(UnaryFunction *f);
  AngleTripletScore() {}
  virtual double evaluate_index(Model *m,
                                const ParticleIndexTriplet &pi,
                                DerivativeAccumulator *da) const override;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override;
  IMP_TRIPLET_SCORE_METHODS(AngleTripletScore);
  IMP_OBJECT_METHODS(AngleTripletScore);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_ANGLE_TRIPLET_SCORE_H */
