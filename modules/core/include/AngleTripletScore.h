/**
 *  \file IMP/core/AngleTripletScore.h
 *  \brief A Score on the angle between three particles.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_ANGLE_TRIPLET_SCORE_H
#define IMPCORE_ANGLE_TRIPLET_SCORE_H

#include <memory>
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

  template<class Archive> void save(Archive &ar) const {
    std::unique_ptr<UnaryFunction> f(f_.get());
    ar(cereal::base_class<TripletScore>(this), f);
    f.release();
  }

  template<class Archive> void load(Archive &ar) {
    std::unique_ptr<UnaryFunction> f;
    ar(cereal::base_class<TripletScore>(this), f);
    f_ = f.release();
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

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(
      IMP::core::AngleTripletScore, cereal::specialization::member_load_save);

#endif /* IMPCORE_ANGLE_TRIPLET_SCORE_H */
