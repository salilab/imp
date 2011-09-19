/**
 *  \file MonteCarlo.cpp  \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/MonteCarlo.h>

#include <IMP/random.h>
#include <IMP/Model.h>
#include <IMP/ConfigurationSet.h>
#include <IMP/dependency_graph.h>

#include <limits>
#include <cmath>
#include <boost/scoped_ptr.hpp>

IMPCORE_BEGIN_NAMESPACE

Mover::Mover(std::string name):Object(name) {}

IMP_LIST_IMPL(MonteCarlo, Mover, mover, Mover*, Movers);

MonteCarlo::MonteCarlo(Model *m): Optimizer(m, "MonteCarlo%1%"),
                                  temp_(1),
                      max_difference_(std::numeric_limits<double>::max()),
                                  probability_(1),
                                  stat_forward_steps_taken_(0),
                                  stat_upward_steps_taken_(0),
                                  stat_num_failures_(0),
                                  return_best_(true),
                                  rand_(0,1), eval_incremental_(false),
                                  incremental_restraint_evals_(0),
                                  incremental_evals_(0){}

bool MonteCarlo::do_accept_or_reject_move(double score, double last) {
  bool ok=false;
  if  (score < last) {
    ok=true;
    if (score < best_energy_ && return_best_) {
      best_= new Configuration(get_model());
      best_energy_=score;
    }
  } else {
    double diff= score- last;
    double e= std::exp(-diff/temp_);
    double r= rand_(random_number_generator);
    IMP_LOG(VERBOSE, diff << " " << temp_ << " " << e << " " << r
            << std::endl);
    if (e > r) {
      ++stat_upward_steps_taken_;
      ok=true;
    } else {
      ok=false;
    }
  }
  if (ok) {
    IMP_LOG(VERBOSE, "Accept: " << score
            << " previous score was " << last << std::endl);
    ++stat_forward_steps_taken_;
    last_energy_=score;
    update_states();
    return true;
  } else {
    IMP_LOG(VERBOSE, "Reject: " << score
            << " current score stays " << last << std::endl);
    for (int i= get_number_of_movers()-1; i>=0; --i) {
      get_mover(i)->reset_move();
    }
    ++stat_num_failures_;
    if (get_use_incremental_evaluate()) {
      rollback_incremental();
    }
    return false;
  }
}

ParticlesTemp MonteCarlo::do_move(double probability) {
  ParticlesTemp ret;
  for (MoverIterator it = movers_begin(); it != movers_end(); ++it) {
    IMP_LOG(VERBOSE, "Moving using " << (*it)->get_name() << std::endl);
    IMP_CHECK_OBJECT(*it);
    {
      IMP_LOG_CONTEXT("Mover " << (*it)->get_name());
      ParticlesTemp cur=(*it)->propose_move(probability);
      ret.insert(ret.end(), cur.begin(), cur.end());
    }
    IMP_LOG(VERBOSE, "end\n");
  }
  return ret;
}

void MonteCarlo::do_step() {
  ParticlesTemp moved=do_move(probability_);
  double energy= do_evaluate(moved);
  do_accept_or_reject_move(energy);
}

double MonteCarlo::do_optimize(unsigned int max_steps) {
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(this);
  if (get_number_of_movers() ==0) {
    IMP_THROW("Running MonteCarlo without providing any"
              << " movers isn't very useful.",
              ValueException);
  }
  if (get_use_incremental_evaluate()) {
    setup_incremental();
  }
  // provide a way of feeding in this value
  last_energy_ =do_evaluate(get_model()->get_particles());
  if (return_best_) {
    best_= new Configuration(get_model());
    best_energy_= last_energy_;
  }
  stat_forward_steps_taken_ = 0;
  stat_num_failures_ = 0;
  update_states();

  IMP_LOG(TERSE, "MC Initial energy is " << last_energy_ << std::endl);

  for (unsigned int i=0; i< max_steps; ++i) {
    if (get_stop_on_good_score() && get_model()->get_has_good_score()) {
      break;
    }
    do_step();
  }

  if (get_use_incremental_evaluate()) {
    teardown_incremental();
  }

  IMP_LOG(TERSE, "MC Final energy is " << last_energy_  << std::endl);
  if (return_best_) {
    //std::cout << "Final score is " << get_model()->evaluate(false)
    //<< std::endl;
    best_->swap_configuration();
    IMP_LOG(TERSE, "MC Returning energy " << best_energy_ << std::endl);
    IMP_IF_CHECK(USAGE) {
      IMP_CHECK_CODE(double e= do_evaluate(get_model()->get_particles()));
      IMP_LOG(TERSE, "MC Got " << e << std::endl);
      IMP_INTERNAL_CHECK(e >= std::numeric_limits<double>::max()
                         && best_energy_ >= std::numeric_limits<double>::max()
                         || std::abs(best_energy_ - e)
                         < .01+.1* std::abs(best_energy_ +e),
                         "Energies do not match "
                         << best_energy_ << " vs " << e << std::endl);
    }
    return best_energy_;
  } else {
    return last_energy_;
  }
}

void MonteCarlo::do_show(std::ostream &) const {
}



void MonteCarlo::setup_incremental() {
  IMP_OBJECT_LOG;
  IMP_LOG(TERSE, "Setting up incremental evaluation." << std::endl);
  RestraintsTemp base= get_restraints();
  for (unsigned int i=0; i< base.size(); ++i) {
    Pointer<Restraint> cur= base[i]->create_incremental_decomposition(1);
    RestraintsTemp curf= IMP::get_restraints(RestraintsTemp(1, cur));
    flattened_restraints_.insert(flattened_restraints_.end(),
                                 curf.begin(), curf.end());
  }
  incremental_scores_
    = get_model()->evaluate(get_as<RestraintsTemp>(flattened_restraints_),
                            false);
  DependencyGraph dg
    = get_dependency_graph(get_as<RestraintsTemp>(flattened_restraints_));
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
      IMP_INTERNAL_CHECK(index.find(cur[j]) != index.end(),
                         "Cannot find restraints " << cur[j]->get_name()
                         << " in index");
      incremental_used_[pi].push_back(index.find(cur[j])->second);
      IMP_LOG(VERBOSE, "Restraint " << cur[j]->get_name()
              << " depends on particle " << ap[i]->get_name() << std::endl);
    }
  }
  IMP_LOG(TERSE, "Done setting up incremental evaluation." << std::endl);
}
void MonteCarlo::teardown_incremental() {
  flattened_restraints_.clear();
  incremental_scores_.clear();
  incremental_used_.clear();
}

void MonteCarlo::rollback_incremental() {
  IMP_INTERNAL_CHECK(old_incremental_scores_.size()
                     == old_incremental_score_indexes_.size(),
                     "Sizes don't match in rollback");
  for (unsigned int i=0; i< old_incremental_scores_.size(); ++i) {
    incremental_scores_[old_incremental_score_indexes_[i]]
      = old_incremental_scores_[i];
  }
}

double MonteCarlo::evaluate_incremental(const ParticleIndexes &moved) const {
  IMP_OBJECT_LOG;
  if (moved.empty()) {
    IMP_LOG(TERSE, "Nothing changed for evaluate"<< std::endl);
  }
  ++incremental_evals_;
  Ints allr;
  for (unsigned int i=0; i< moved.size(); ++i) {
    allr.insert(allr.end(), incremental_used_[moved[i]].begin(),
                incremental_used_[moved[i]].end());
  }
  std::sort(allr.begin(), allr.end());
  allr.erase(std::unique(allr.begin(), allr.end()), allr.end());
  RestraintsTemp curr(allr.size());
  for (unsigned int i=0; i< allr.size(); ++i) {
    curr[i]= flattened_restraints_[allr[i]];
  }
  IMP_USAGE_CHECK(moved.empty()== curr.empty(),
                  "Particles were moved but no restraints were found: "
                  << IMP::internal::get_particle(get_model(), moved));
  incremental_restraint_evals_+= curr.size();
  Floats scores= get_model()->evaluate(curr, false);
  old_incremental_scores_.resize(allr.size());
  //old_incremenal_scores_indexes_.resize(incremental_scores_.size());
  for (unsigned int i=0; i< allr.size(); ++i) {
    old_incremental_scores_[i]= incremental_scores_[allr[i]];
    incremental_scores_[allr[i]]=scores[i];
  }
  using std::swap;
  using IMP::operator<<;
  swap(old_incremental_score_indexes_, allr);
  double ret= std::accumulate(incremental_scores_.begin(),
                              incremental_scores_.end(), 0.0);
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    for (unsigned int i=0; i< flattened_restraints_.size(); ++i) {
      double cur= flattened_restraints_[i]->evaluate(false);
      IMP_INTERNAL_CHECK(std::abs(cur-incremental_scores_[i])
                         < .1*std::abs(cur+incremental_scores_[i])+.1,
                         "Scores don't match for restraint "
                         << flattened_restraints_[i]->get_name()
                         << " have " << incremental_scores_[i]
                         << " but got " << cur << " when moving "
                         << moved << " and evaluating "
                         <<  curr);
    }
  }
  return ret;
}

double MonteCarlo::evaluate_incremental_if_below(const ParticleIndexes &,
                                     double ) const {
  IMP_NOT_IMPLEMENTED;
}




MonteCarloWithLocalOptimization::MonteCarloWithLocalOptimization(Optimizer *opt,
                                                           unsigned int steps):
  MonteCarlo(opt->get_model()), opt_(opt), num_local_(steps) {}


void MonteCarloWithLocalOptimization::do_step() {
  ParticlesTemp moved=do_move(get_move_probability());
  IMP_LOG(TERSE,
          "MC Performing local optimization from "
          << do_evaluate(moved) << std::endl);
  // non-Mover parts of the model can be moved by the local optimizer
  // make sure they are cleaned up
  Pointer<Configuration> cs= new Configuration(get_model());
  double ne =opt_->optimize(num_local_);
  if (!do_accept_or_reject_move(ne)) {
    cs->swap_configuration();
  }
}

void MonteCarloWithLocalOptimization::do_show(std::ostream &) const {
}



MonteCarloWithBasinHopping::MonteCarloWithBasinHopping(Optimizer *opt,
                                                       unsigned int steps):
  MonteCarloWithLocalOptimization(opt, steps) {}


void MonteCarloWithBasinHopping::do_step() {
  ParticlesTemp moved=do_move(get_move_probability());
  IMP_LOG(TERSE,
          "MC Performing local optimization from "
          << do_evaluate(moved) << std::endl);
  Pointer<Configuration> cs= new Configuration(get_model());
  double ne =get_local_optimizer()->optimize(get_number_of_steps());
  cs->swap_configuration();
  do_accept_or_reject_move(ne);
}

void MonteCarloWithBasinHopping::do_show(std::ostream &) const {
}


IMPCORE_END_NAMESPACE
