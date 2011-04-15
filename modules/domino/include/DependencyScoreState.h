/**
 *  \file domino/DependencyScoreState.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_DEPENDENCY_SCORE_STATE_H
#define IMPDOMINO_DEPENDENCY_SCORE_STATE_H

#include "domino_config.h"
#include <IMP/ScoreState.h>

IMPDOMINO_BEGIN_NAMESPACE



//! Add a dependency to the dependency graph
/** This score state doesn't do anything other than add edges to the
    dependency graph. This can be useful when involved filters are used
    as they are not included in the dependency graph. This may go away
    and be replaced by adding dependencies to the SubsetFilterTable.
 */
class IMPDOMINOEXPORT DependencyScoreState : public ScoreState
{
  ParticlesTemp inputp_, outputp_;
  ContainersTemp inputc_, outputc_;
public:
  DependencyScoreState();
  void set_input_particles(const ParticlesTemp &pt) {
    IMP_USAGE_CHECK(!get_has_model(),
                    "Must set dependencies before adding to model.");
    inputp_=pt;
  }
  void set_output_particles(const ParticlesTemp &pt) {
    IMP_USAGE_CHECK(!get_has_model(),
                    "Must set dependencies before adding to model.");
    outputp_=pt;
  }
  void set_input_containers(const ContainersTemp &pt) {
    IMP_USAGE_CHECK(!get_has_model(),
                    "Must set dependencies before adding to model.");
    inputc_=pt;
  }
  void set_output_containers(const ContainersTemp &pt) {
    IMP_USAGE_CHECK(!get_has_model(),
                    "Must set dependencies before adding to model.");
    outputc_=pt;
  }
  IMP_SCORE_STATE(DependencyScoreState);
};


IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_DEPENDENCY_SCORE_STATE_H */
