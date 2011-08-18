/**
 *  \file Optimizer.cpp   \brief Base class for all optimizers.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/log.h"
#include "IMP/Optimizer.h"
#include "IMP/OptimizerState.h"
#include "IMP/RestraintSet.h"
#include "IMP/dependency_graph.h"
#include "IMP/internal/utility.h"
#include <boost/tuple/tuple.hpp>
#include <limits>
#include <algorithm>

IMP_BEGIN_NAMESPACE

Optimizer::Optimizer(Model *m, std::string name): Object(name)
{
  if (m) set_model(m);
  set_was_used(true);
  min_score_= -std::numeric_limits<double>::max();
  stop_on_good_score_=false;
  last_score_= std::numeric_limits<double>::max();
  eval_incremental_=false;
}

Optimizer::~Optimizer()
{
}

void Optimizer::update_states() const
{
  IMP_LOG(VERBOSE,
          "Updating OptimizerStates " << std::flush);
  for (OptimizerStateConstIterator it = optimizer_states_begin();
       it != optimizer_states_end(); ++it) {
    IMP_CHECK_OBJECT(*it);
    (*it)->update();
    IMP_LOG(VERBOSE, "." << std::flush);
  }
  IMP_LOG(VERBOSE, "done." << std::endl);
}

double Optimizer::optimize(unsigned int max_steps) {
  IMP_FUNCTION_LOG;
  if (!model_) {
    IMP_THROW("Must give the optimizer a model to optimize",
              ValueException);
  }
  if (eval_incremental_) {
    setup_incremental();
  } else {
    flattened_restraints_
    =IMP::get_restraints(RestraintsTemp(restraints_.begin(),
                                        restraints_.end()));
  }
  set_was_used(true);

  double ret= do_optimize(max_steps);
  if (eval_incremental_) {
    teardown_incremental();
  }
  return ret;
}

IMP_LIST_IMPL(Optimizer, OptimizerState, optimizer_state,
              OptimizerState*, OptimizerStates);

void Optimizer::set_optimizer_state_optimizer(OptimizerState *os, Optimizer *o)
{
  os->set_optimizer(o);
}


void Optimizer::set_restraints(const RestraintsTemp &rs) {
  restraints_=Restraints(rs.begin(), rs.end());
}

double Optimizer::evaluate(bool compute_derivatives) const {
  IMP_FUNCTION_LOG;
  if (restraints_.empty()) {
    last_score_= get_model()->evaluate(compute_derivatives);
  } else {
    IMP::Floats ret= get_model()->evaluate(flattened_restraints_,
                                           compute_derivatives);
    last_score_= std::accumulate(ret.begin(), ret.end(), 0.0);
  }
  return last_score_;
}

double Optimizer::evaluate_if_below(bool compute_derivatives,
                                   double max) const {
  IMP_FUNCTION_LOG;
  RestraintsTemp rs= flattened_restraints_;
  if (rs.empty()) {
    rs
      = IMP::get_restraints(RestraintsTemp(1, get_model()
                                           ->get_root_restraint_set()));
  }
  IMP::Floats ret= get_model()->evaluate_if_below(rs,
                                                  compute_derivatives, max);
  last_score_= std::accumulate(ret.begin(), ret.end(), 0.0);
  return last_score_;
}


RestraintsTemp Optimizer::get_restraints() const {
  if (restraints_.empty()) {
    return RestraintsTemp(1, model_->get_root_restraint_set());
  } else {
    return restraints_;
  }
}

void Optimizer::setup_incremental() {
  IMP_OBJECT_LOG;
  IMP_LOG(TERSE, "Setting up incremental evaluation." << std::endl);
  flattened_restraints_=create_decomposition(get_restraints());
  for (unsigned int i=0; i< flattened_restraints_.size(); ++i) {
    get_model()->add_temporary_restraint(flattened_restraints_[i]);
  }
  incremental_scores_= get_model()->evaluate(flattened_restraints_, false);
  DependencyGraph dg= get_dependency_graph(flattened_restraints_);
  compatibility::map<Restraint*, int> index;
  for (unsigned int i=0; i< flattened_restraints_.size(); ++i) {
    index[flattened_restraints_[i]]=i;
  }
  ParticlesTemp ap= get_model()->get_particles();
  for (unsigned int i=0; i< ap.size(); ++i) {
    RestraintsTemp cur=get_dependent_restraints(ap[i], ParticlesTemp(),
                                                dg);
    ParticleIndex pi= ap[i]->get_index();
    incremental_used_.resize(std::max<unsigned int>(incremental_used_.size(),
                                      pi+1));
    for (unsigned int j=0; j< cur.size(); ++j) {
      if (index.find(cur[j]) != index.end()) {
        incremental_used_[pi].push_back(index.find(cur[j])->second);
      }
    }
  }
  IMP_LOG(TERSE, "Done setting up incremental evaluation." << std::endl);
}
void Optimizer::teardown_incremental() {
  flattened_restraints_.clear();
  incremental_scores_.clear();
  incremental_used_.clear();
}

double Optimizer::evaluate_incremental(const ParticleIndexes &moved) const {
  Ints allr;
  for (unsigned int i=0; i< moved.size(); ++i) {
    allr.insert(allr.end(), incremental_used_[i].begin(),
                incremental_used_[i].end());
  }
  RestraintsTemp cur(allr.size());
  for (unsigned int i=0; i< allr.size(); ++i) {
    cur[i]= flattened_restraints_[allr[i]];
  }
  Floats score= get_model()->evaluate(cur, false);
  for (unsigned int i=0; i< allr.size(); ++i) {
    incremental_scores_[allr[i]]=score[i];
  }
  return std::accumulate(incremental_scores_.begin(),
                         incremental_scores_.end(), 0);
}

double Optimizer::evaluate_incremental_if_below(const ParticleIndexes &,
                                     double ) const {
  IMP_NOT_IMPLEMENTED;
}

IMP_END_NAMESPACE
