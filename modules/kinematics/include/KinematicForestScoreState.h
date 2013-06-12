/**
 * \file KinematicForestScoreState.h
 * \brief
 *
 * \authors Dina Schneidman, Barak Raveh
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKINEMATICS_KINEMATIC_FOREST_SCORE_STATE_H
#define IMPKINEMATICS_KINEMATIC_FOREST_SCORE_STATE_H

#include "kinematics_config.h"
#include <IMP/ScoreState.h>
#include <IMP/base/warning_macros.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/kinematics/KinematicForest.h>

IMPKINEMATICS_BEGIN_NAMESPACE

namespace {
Model *extract_model(const IMP::core::RigidBodies& rbs,
                     const IMP::ParticlesTemp& atoms) {
  if (!rbs.empty()) return rbs[0].get_model();
  else if (!atoms.empty()) return atoms[0]->get_model();
  else {
    IMP_FAILURE("No particles passed to KinematicForestScoreState");
  }
}
}

class IMPKINEMATICSEXPORT KinematicForestScoreState : public IMP::ScoreState {
 public:

  IMP_OBJECT_METHODS(KinematicForestScoreState);

  KinematicForestScoreState(KinematicForest *kf,
                            IMP::core::RigidBodies rbs,
                            IMP::ParticlesTemp atoms) :
      ScoreState(extract_model(rbs, atoms),
                 "KinematicForestScoreState%1%"),
    kf_(kf), rbs_(rbs), atoms_(atoms) {}

  // functions that ScoreState requires
  void do_before_evaluate() {
    kf_->update_all_external_coordinates();
  }

  void do_after_evaluate(DerivativeAccumulator *da) { IMP_UNUSED(da); }

  ModelObjectsTemp do_get_inputs() const;

  ModelObjectsTemp do_get_outputs() const;

 private:
  KinematicForest *kf_;
  IMP::core::RigidBodies rbs_;
  IMP::ParticlesTemp atoms_;
};

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_KINEMATIC_FOREST_SCORE_STATE_H */
