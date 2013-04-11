/**
 *  \file MonteCarlo.cpp  \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
                                const RestraintsTemp &all,
                                std::string name):
    IMP::internal::RestraintsScoringFunction(IMP::internal::get_model(all),
                                             1.0, NO_MAX, name),
    pi_(pi), all_restraints_(all) {}

namespace {
std::pair<Ints, Restraints> get_my_restraints(Particle *p,
                       const base::map<Restraint*, int> &all,
                                              Model *m) {
  RestraintsTemp cr
    = get_dependent_restraints(p, ParticlesTemp(),
                               m->get_dependency_graph(),
                               m->get_dependency_graph_vertex_index());
  Restraints mr;
  Ints mi;
  for (unsigned int j=0; j < cr.size(); ++j) {
    if (all.find(cr[j]) != all.end()) {
      mi.push_back(all.find(cr[j])->second);
      IMP_INTERNAL_CHECK(std::find(mr.begin(), mr.end(), cr[j]) == mr.end(),
                         "Found duplicate restraint " << Showable(cr[j])
                         << " in list " << cr);
      mr.push_back(cr[j]);
    }
  }
  IMP_LOG_TERSE( "Particle " << Showable(p) << " has restraints "
          << mr << std::endl);
  return std::make_pair(mi, mr);
}

}

void
SingleParticleScoringFunction::do_update_dependencies() {
  IMP_OBJECT_LOG;
  base::map<Restraint*, int> mp;
  IMP_LOG_TERSE( "All restraints are " << all_restraints_ << std::endl);
  for (unsigned int i=0; i< all_restraints_.size(); ++i) {
    mp[all_restraints_[i]]= i;
  }
  Restraints mr;
  boost::tie(indexes_, mr)
    = get_my_restraints(get_model()->get_particle(pi_),
                        mp,
                        get_model());
  IMP_LOG_TERSE( "Found " << mr << " for particle "
          << Showable(get_model()->get_particle(pi_)) << std::endl);
  // so that model dependencies are not reset
  IMP::internal::RestraintsScoringFunction::set_restraints(mr);
  IMP::internal::RestraintsScoringFunction::do_update_dependencies();
}


ScoreStatesTemp
SingleParticleScoringFunction
::get_required_score_states() const {
  IMP_OBJECT_LOG;
  ScoreStatesTemp from_restraints
      =IMP::internal::RestraintsScoringFunction::get_required_score_states();
  IMP_LOG_TERSE( "Score states from restraints are " << from_restraints
          << "(" << IMP::internal::RestraintsScoringFunction::get_restraints()
          << ")" << std::endl);
  ScoreStatesTemp from_dummy;
  for (unsigned int i=0; i< dummy_restraints_.size(); ++i) {
    from_dummy+= get_model()->get_required_score_states(dummy_restraints_[i]);
  }
  IMP_LOG_TERSE( "Score states from dummy are " << from_dummy
          << "(" << dummy_restraints_ << ")" << std::endl);
  ScoreStatesTemp deps
      = IMP::get_dependent_score_states(get_model()->get_particle(pi_),
                                        ModelObjectsTemp(),
                                        get_model()->get_dependency_graph(),
                            get_model()->get_dependency_graph_vertex_index());
  IMP_LOG_TERSE( "Dependent score states are " << deps << std::endl);
  std::sort(deps.begin(), deps.end());
  ScoreStatesTemp allin= from_restraints+from_dummy;
  std::sort(allin.begin(), allin.end());
  // intersect the lists to determine which depend on this particle and are need
  ScoreStatesTemp ret;
  std::set_intersection(allin.begin(), allin.end(),
                        deps.begin(), deps.end(),
                        std::back_inserter(ret));
  IMP_LOG_TERSE( "Particle " << Showable(get_model()->get_particle(pi_))
          << " will update " << get_update_order(ret) << std::endl);
  return get_update_order(ret);
}


void
SingleParticleScoringFunction
::add_dummy_restraint(Restraint *r) {
  IMP_USAGE_CHECK(r, "nullptr dummy restraint.");
  get_model()->clear_caches();
  dummy_restraints_.push_back(r);
}

void SingleParticleScoringFunction
::clear_dummy_restraints() {
  dummy_restraints_.clear();
  get_model()->clear_caches();
}


IMPCORE_END_INTERNAL_NAMESPACE
