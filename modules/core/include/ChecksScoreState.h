/**
 *  \file IMP/core/ChecksScoreState.h    \brief Turn checks on and off.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_CHECKS_SCORE_STATE_H
#define IMPCORE_CHECKS_SCORE_STATE_H

#include <IMP/core/core_config.h>
#include <IMP/macros.h>
#include <IMP/Particle.h>
#include <IMP/ScoreState.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPCORE_BEGIN_NAMESPACE

/** Turn checks on with a given probability each evaluate call.
 */
class IMPCOREEXPORT ChecksScoreState : public ScoreState {
  double probability_;
  unsigned int num_checked_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<ScoreState>(this), probability_, num_checked_);
  }
  IMP_OBJECT_SERIALIZE_DECL(ChecksScoreState);

 public:
  ChecksScoreState(Model *m, double probability);
  ChecksScoreState() {}

  unsigned int get_number_of_checked() const { return num_checked_; }

  virtual void do_before_evaluate() override;
  virtual void do_after_evaluate(DerivativeAccumulator *da) override;
  virtual ModelObjectsTemp do_get_outputs() const override;
  virtual ModelObjectsTemp do_get_inputs() const override;
  IMP_OBJECT_METHODS(ChecksScoreState);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CHECKS_SCORE_STATE_H */
