/**
 *  \file IMP/domino/DependencyScoreState.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_DEPENDENCY_SCORE_STATE_H
#define IMPDOMINO_DEPENDENCY_SCORE_STATE_H

#include <IMP/domino/domino_config.h>
#include <IMP/ScoreState.h>
#include <IMP/score_state_macros.h>

IMPDOMINO_BEGIN_NAMESPACE

//! Add a dependency to the dependency graph
/** This score state doesn't do anything other than add edges to the
    dependency graph. This can be useful when involved filters are used
    as they are not included in the dependency graph. This may go away
    and be replaced by adding dependencies to the SubsetFilterTable.
 */
class IMPDOMINOEXPORT DependencyScoreState : public ScoreState {
  ParticlesTemp inputp_, outputp_;
  ContainersTemp inputc_, outputc_;

 public:
  DependencyScoreState();
  void set_input_particles(const ParticlesTemp &pt) {
    IMP_USAGE_CHECK(!get_is_part_of_model(),
                    "Must set dependencies before adding to model.");
    inputp_ = pt;
  }
  void set_output_particles(const ParticlesTemp &pt) {
    IMP_USAGE_CHECK(!get_is_part_of_model(),
                    "Must set dependencies before adding to model.");
    outputp_ = pt;
  }
  void set_input_containers(const ContainersTemp &pt) {
    IMP_USAGE_CHECK(!get_is_part_of_model(),
                    "Must set dependencies before adding to model.");
    inputc_ = pt;
  }
  void set_output_containers(const ContainersTemp &pt) {
    IMP_USAGE_CHECK(!get_is_part_of_model(),
                    "Must set dependencies before adding to model.");
    outputc_ = pt;
  }

 protected:
  virtual void do_before_evaluate() IMP_OVERRIDE;
  virtual void do_after_evaluate(DerivativeAccumulator *da) IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(DependencyScoreState);
  ;
};

IMPDOMINO_END_NAMESPACE

#endif /* IMPDOMINO_DEPENDENCY_SCORE_STATE_H */
