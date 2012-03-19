/**
 *  \file MonteCarlo.cpp  \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/IncrementalScoringFunction.h>
#include <IMP/RestraintSet.h>
#include <IMP/Restraint.h>
#include <IMP/dependency_graph.h>
#include <IMP/compatibility/set.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/XYZR.h>
#include <numeric>
IMPCORE_BEGIN_NAMESPACE
/** to handle good/max evaluate, add dummy restraints for each
    restraint set that return 0 or inf if the last scores for the
    set are bad.*/


IncrementalScoringFunction::SingleParticleScoringFunction
::SingleParticleScoringFunction(ParticleIndex pi,
                                const RestraintsTemp &rs,
                                const Ints &indexes):
  RestraintsScoringFunction(rs),
  indexes_(indexes), pi_(pi) {}

const ScoreStatesTemp
IncrementalScoringFunction::SingleParticleScoringFunction
::get_extra_score_states(const DependencyGraph &dg) const {
  return IMP::get_dependent_score_states(get_model()->get_particle(pi_),
                                         ParticlesTemp(),
                                         dg);
}

IncrementalScoringFunction
::IncrementalScoringFunction(const RestraintsTemp &rs):
  ScoringFunction(rs[0]->get_model(),
                  "IncrementalScoringFunction%1%") {
  Pointer<ScoringFunction> avoid(this);
  create_flattened_restraints(rs);
  create_scoring_functions();
  // suppress check error
  initialize_scores();
  avoid.release();
  moved_=base::get_invalid_index<ParticleIndexTag>();
}
void IncrementalScoringFunction::create_scoring_functions() {
  if (flattened_restraints_.empty()) return;
  compatibility::map<Restraint*,int> all_set;
  for (unsigned int i=0; i< flattened_restraints_.size(); ++i) {
    all_set[flattened_restraints_[i]]=i;
  }
  DependencyGraph dg
    = get_dependency_graph(flattened_restraints_[0]->get_model());
  for (Model::ParticleIterator it=get_model()->particles_begin();
       it != get_model()->particles_end(); ++it) {
    RestraintsTemp cr= get_dependent_restraints(*it, ParticlesTemp(),
                                                dg);
    RestraintsTemp mr;
    Ints mi;
    for (unsigned int i=0; i < cr.size(); ++i) {
      if (all_set.find(cr[i]) != all_set.end()) {
        mr.push_back(all_set.find(cr[i])->first);
        mi.push_back(all_set.find(cr[i])->second);
      }
    }
    if (!mr.empty()) {
      scoring_functions_[(*it)->get_index()]
        = new SingleParticleScoringFunction((*it)->get_index(),
                                            mr, mi);
    }
  }
}
void IncrementalScoringFunction::initialize_scores() {
  IMP_NEW(RestraintsScoringFunction, sf, (flattened_restraints_));
  sf->evaluate(false);
  flattened_restraints_scores_.resize(flattened_restraints_.size());
  for (unsigned int i=0; i < flattened_restraints_.size(); ++i) {
    flattened_restraints_scores_[i]
      = flattened_restraints_[i]->get_last_score();
  }
  for (unsigned int i=0; i< nbl_.size(); ++i) {
    nbl_[i].initialize();
  }
}
void IncrementalScoringFunction
::create_flattened_restraints(const RestraintsTemp &rs) {
  Restraints decomposed;
  for (unsigned int i=0; i < rs.size(); ++i) {
    Pointer<Restraint> cur= rs[i]->create_decomposition();
    if (cur) {
      decomposed.push_back(cur);
    }
  }
  flattened_restraints_=IMP::get_restraints(decomposed.begin(),
                                                    decomposed.end());

}
void IncrementalScoringFunction::reset() {
  initialize_scores();
}
void IncrementalScoringFunction::reset_moved_particles() {
  moved_=base::get_invalid_index<ParticleIndexTag>();
  rollback();
}
void IncrementalScoringFunction::set_moved_particles(const ParticlesTemp &p) {
  IMP_USAGE_CHECK(p.size()<=1, "Can only move one particle at a time");
  if (p.empty()) {
    moved_=base::get_invalid_index<ParticleIndexTag>();
  } else moved_=p[0]->get_index();
}
// make sure to reset last scores
void IncrementalScoringFunction::rollback() {
  IMP_OBJECT_LOG;
  for (unsigned int i=0; i< old_incremental_score_indexes_.size(); ++i) {
    int index=old_incremental_score_indexes_[i];
    IMP_LOG(TERSE, "Rolling back score for "
            << Showable(flattened_restraints_[index]) << " from "
                << flattened_restraints_scores_[index]
                << " to " << old_incremental_scores_[i] << std::endl);

    flattened_restraints_scores_[index]
      = old_incremental_scores_[i];
  }
  for (unsigned int i=0; i< nbl_.size(); ++i) {
    nbl_[i].rollback();
  }
}
void IncrementalScoringFunction::add_close_pair_score(PairScore *ps,
                                                      double distance,
                                                      const ParticlesTemp
                                                      &particles,
                                                      const PairFilters &
                                                      filters) {
  nbl_.push_back(NBLScore(ps, distance, particles, filters));
}


IncrementalScoringFunction::NBLScore::NBLScore(PairScore *ps,
                                               double distance,
                                               const ParticlesTemp &particles,
                                               const PairFilters &filters) {
  m_= IMP::internal::get_model(particles);
  score_=ps;
  double maxr=0;
  for (unsigned int i=0; i< particles.size(); ++i) {
    maxr=std::max(maxr, core::XYZR(particles[i]).get_radius());
  }
  distance_=distance+2*maxr;
  pis_= IMP::internal::get_index(particles);
  filters_=filters;

  algebra::Vector3Ds vs(pis_.size());
  for (unsigned int i=0; i< pis_.size(); ++i) {
    vs[i]= XYZ(m_, pis_[i]).get_coordinates();
  }
  dnn_= new algebra::DynamicNearestNeighbor3D(vs, distance_);
}

void IncrementalScoringFunction::NBLScore::initialize() {
  for (unsigned  int i=0; i< pis_.size(); ++i) {
    Ints n= dnn_->get_in_ball(i, distance_);
    for (unsigned int j=0; j< n.size(); ++j) {
      if (n[j] < static_cast<int>(i)) {
        ParticleIndex o= pis_[n[j]];
        ParticleIndexPair pp(pis_[i], o);
        double score= score_->evaluate_index(m_, pp, nullptr);
        cache_.insert(Score(pis_[i], o, score));
      }
    }
  }
}

void IncrementalScoringFunction::NBLScore::remove_score(Score pr) {
  Hash0Iterator b,e;
  boost::tie(b,e)=cache_.get<0>().equal_range(pr.i0);
  for (Hash0Iterator c=b; c!= e; ++c){
    if (c->i1==pr.i1) {
      cache_.erase(c);
      break;
    }
  }
}



void IncrementalScoringFunction::NBLScore::rollback() {
  for (unsigned int i=0; i< added_.size(); ++i) {
    remove_score(added_[i]);
  }
  for (unsigned int i=0; i < removed_.size(); ++i) {
    cache_.insert(Score(removed_[i]));
  }
  added_.clear();
  removed_.clear();
}

void IncrementalScoringFunction::NBLScore::cleanup_score(ParticleIndex moved) {
  {
    Hash0Iterator b,e;
    boost::tie(b,e)=cache_.get<0>().equal_range(moved);
    removed_.insert(removed_.end(), b,e);
    cache_.get<0>().erase(b,e);
  }
  {
    Hash1Iterator b,e;
    boost::tie(b,e)=cache_.get<1>().equal_range(moved);
    removed_.insert(removed_.end(), b,e);
    cache_.get<1>().erase(b,e);
  }
}

void IncrementalScoringFunction::NBLScore::fill_scores(ParticleIndex moved) {
  XYZ d(m_, moved);
  int id=to_dnn_.find(moved)->second;
  dnn_->set_coordinates(id, d.get_coordinates());
  Ints n= dnn_->get_in_ball(id, distance_);
  for (unsigned int i=0; i< n.size(); ++i) {
    ParticleIndex o= pis_[n[i]];
    ParticleIndexPair pp(moved, o);
    double score= score_->evaluate_index(m_, pp, nullptr);
    Score s(moved, o, score);
    cache_.insert(s);
    added_.push_back(s);
  }
}

double IncrementalScoringFunction::NBLScore::get_score(ParticleIndex moved) {
  added_.clear();
  removed_.clear();
  if (moved!=base::get_invalid_index<ParticleIndexTag>()) {
    cleanup_score(moved);
    fill_scores(moved);
  }
  return std::accumulate(cache_.begin(), cache_.end(), 0.0);
}


ScoringFunction::ScoreIsGoodPair
IncrementalScoringFunction::do_evaluate_if_good(bool ,
                                    const ScoreStatesTemp &) {
  IMP_NOT_IMPLEMENTED;
}
ScoringFunction::ScoreIsGoodPair
IncrementalScoringFunction::do_evaluate(bool derivatives,
                                         const ScoreStatesTemp &ss) {
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(ss.empty(), "Where did the score states come from?");
  old_incremental_scores_.clear();
  old_incremental_score_indexes_.clear();
  IMP_LOG(TERSE, "Evaluate with " << moved_ << std::endl);
  if (moved_ != base::get_invalid_index<ParticleIndexTag>()) {
    ScoringFunctionsMap::const_iterator it=scoring_functions_.find(moved_);
    if (it != scoring_functions_.end()) {
      it->second->evaluate(derivatives);
      old_incremental_score_indexes_=it->second->get_restraint_indexes();
      old_incremental_scores_.resize(old_incremental_score_indexes_.size());
      for (unsigned int i=0; i< old_incremental_score_indexes_.size(); ++i) {
        int index=old_incremental_score_indexes_[i];
        old_incremental_scores_[i]=flattened_restraints_scores_[index];
        double score=flattened_restraints_[index]->get_last_score();
        IMP_LOG(TERSE, "Updating score for "
                << Showable(flattened_restraints_[index]) << " from "
                << old_incremental_scores_[i]
                << " to " << score << std::endl);
        flattened_restraints_scores_[index]=score;
      }
    }
  }
  IMP_LOG(VERBOSE, "Scores are " << flattened_restraints_scores_ << std::endl);
  double score=std::accumulate(flattened_restraints_scores_.begin(),
                               flattened_restraints_scores_.end(),
                               0.0);
  // do nbl stuff
  for (unsigned int i=0; i< nbl_.size(); ++i) {
    score+= nbl_[i].get_score(moved_);
  }
  return std::make_pair(score, true);
}
ScoringFunction::ScoreIsGoodPair
IncrementalScoringFunction::do_evaluate_if_below(bool ,
                                                 double ,
                                                 const ScoreStatesTemp &) {
  IMP_NOT_IMPLEMENTED;
}
RestraintsTemp IncrementalScoringFunction::get_restraints() const {
  return RestraintsTemp();
}


void IncrementalScoringFunction::do_show(std::ostream &) const {
}

IMPCORE_END_NAMESPACE
