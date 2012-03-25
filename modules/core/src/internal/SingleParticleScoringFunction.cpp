/**
 *  \file MonteCarlo.cpp  \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/internal/incremental_scoring_function.h>
#include <IMP/core/internal/SingleParticleScoringFunction.h>
#include <numeric>
IMPCORE_BEGIN_INTERNAL_NAMESPACE
/** to handle good/max evaluate, add dummy restraints for each
    restraint set that return 0 or inf if the last scores for the
    set are bad.*/


SingleParticleScoringFunction
::SingleParticleScoringFunction(ParticleIndex pi,
                                const RestraintsTemp &rs,
                                const Ints &indexes):
  RestraintsScoringFunction(rs),
  indexes_(indexes), pi_(pi) {}

ScoreStatesTemp
SingleParticleScoringFunction
::get_required_score_states(const DependencyGraph &dg) const {
  IMP_OBJECT_LOG;
  ScoreStatesTemp from_restraints
      =RestraintsScoringFunction::get_required_score_states(dg);
  IMP_LOG(TERSE, "Score states from restraints are " << from_restraints
          << "(" << RestraintsScoringFunction::get_restraints()
          << ")" << std::endl);
  ScoreStatesTemp from_dummy
      = get_model()->get_score_states(dummy_restraints_);
  IMP_LOG(TERSE, "Score states from dummy are " << from_dummy
          << "(" << dummy_restraints_ << ")" << std::endl);
  ScoreStatesTemp deps
      = IMP::get_dependent_score_states(get_model()->get_particle(pi_),
                                        base::ObjectsTemp(), dg);
  IMP_LOG(TERSE, "Dependent score states are " << deps << std::endl);
  std::sort(deps.begin(), deps.end());
  ScoreStatesTemp allin= from_restraints+from_dummy;
  std::sort(allin.begin(), allin.end());
  // intersect the lists to determine which depend on this particle and are need
  ScoreStatesTemp ret;
  std::set_intersection(allin.begin(), allin.end(),
                        deps.begin(), deps.end(),
                        std::back_inserter(ret));
  IMP_LOG(TERSE, "Particle " << Showable(get_model()->get_particle(pi_))
          << " will update " << get_ordered_score_states(ret) << std::endl);
  return get_ordered_score_states(ret);
}


void
SingleParticleScoringFunction
::add_dummy_restraint(Restraint *r) {
  get_model()->reset_dependencies();
  dummy_restraints_.push_back(r);
}

void SingleParticleScoringFunction
::clear_dummy_restraints() {
  dummy_restraints_.clear();
  get_model()->reset_dependencies();
}


IMPCORE_END_INTERNAL_NAMESPACE
