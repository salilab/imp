/**
 *  \file MonteCarlo.cpp  \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/MonteCarlo.h>

#include <IMP/random.h>
#include <IMP/Model.h>
#include <IMP/ConfigurationSet.h>
#include <IMP/core/GridClosePairsFinder.h>
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
    IMP_LOG(TERSE, "Accept: " << score
            << " previous score was " << last << std::endl);
    ++stat_forward_steps_taken_;
    last_energy_=score;
    update_states();
    return true;
  } else {
    IMP_LOG(TERSE, "Reject: " << score
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
      //IMP_LOG_CONTEXT("Mover " << (*it)->get_name());
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

  IMP_LOG(TERSE, "MC Final energy is " << last_energy_  << std::endl);
  if (return_best_) {
    //std::cout << "Final score is " << get_model()->evaluate(false)
    //<< std::endl;
    best_->swap_configuration();
    IMP_LOG(TERSE, "MC Returning energy " << best_energy_ << std::endl);
    IMP_IF_CHECK(USAGE) {
      IMP_CHECK_CODE(double e= do_evaluate(get_model()->get_particles()));
      IMP_LOG(TERSE, "MC Got " << e << std::endl);
      IMP_INTERNAL_CHECK((e >= std::numeric_limits<double>::max()
                          && best_energy_ >= std::numeric_limits<double>::max())
                         || std::abs(best_energy_ - e)
                         < .01+.1* std::abs(best_energy_ +e),
                         "Energies do not match "
                         << best_energy_ << " vs " << e << std::endl);
    }
    // ick
    if (get_use_incremental_evaluate()) {
      teardown_incremental();
    }
    return evaluate(false);
  } else {
    // ick
    if (get_use_incremental_evaluate()) {
      teardown_incremental();
    }
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
  IMP_LOG(VERBOSE, "Restraints flattened into " << flattened_restraints_
          << std::endl);
  RestraintsTemp restraints=get_as<RestraintsTemp>(flattened_restraints_);
  incremental_scores_= get_model()->evaluate(restraints,
                                             false);
  DependencyGraph dg
    = get_dependency_graph(restraints);
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

  // nonbonded
  if (dnn_) {
    IMP_NEW(GridClosePairsFinder, cpf, ());
    cpf->set_distance(nbl_.distance_);
    ParticleIndexPairs pips= cpf->get_close_pairs(get_model(), nbl_.pis_);
    nbl_.initialize(get_model(), pips);
  }


  using std::operator<<;
  using base::operator<<;
  IMP_LOG(TERSE, "Done setting up incremental evaluation. Initial scores are"
          << incremental_scores_ << std::endl);
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

  // nbl
  if (dnn_) {
    dnn_->set_coordinates(to_dnn_[moved_], XYZ(get_model(),
                                               moved_).get_coordinates());
    nbl_.roll_back(get_model(), moved_);
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
    if (moved[i] < static_cast<int>(incremental_used_.size())) {
      allr.insert(allr.end(), incremental_used_[moved[i]].begin(),
                  incremental_used_[moved[i]].end());
    }
  }
  std::sort(allr.begin(), allr.end());
  allr.erase(std::unique(allr.begin(), allr.end()), allr.end());
  RestraintsTemp curr(allr.size());
  for (unsigned int i=0; i< allr.size(); ++i) {
    curr[i]= flattened_restraints_[allr[i]];
  }
  /*IMP_USAGE_CHECK(moved.empty()== curr.empty(),
                  "Particles were moved but no restraints were found: "
                  << IMP::internal::get_particle(get_model(), moved));*/
  incremental_restraint_evals_+= curr.size();
  Floats scores= get_model()->evaluate(curr, false);
  old_incremental_scores_.resize(allr.size());
  //old_incremenal_scores_indexes_.resize(incremental_scores_.size());
  for (unsigned int i=0; i< allr.size(); ++i) {
    IMP_LOG(VERBOSE, "Updating score for "
            << flattened_restraints_[allr[i]]->get_name()
            << " from " << incremental_scores_[allr[i]]
            << " to " << scores[i] << std::endl);
    old_incremental_scores_[i]= incremental_scores_[allr[i]];
    incremental_scores_[allr[i]]=scores[i];
  }
  using std::swap;
  using IMP::operator<<;
  swap(old_incremental_score_indexes_, allr);
  double ret= std::accumulate(incremental_scores_.begin(),
                              incremental_scores_.end(), 0.0);
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    base::SetLogState sls(SILENT);
    for (unsigned int i=0; i< flattened_restraints_.size(); ++i) {
      double cur= flattened_restraints_[i]->evaluate(false);
      IMP_UNUSED(cur);
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
  if (moved.size() >1) {
    // evil hack
    return ret+ nbl_.prior_;
  } else {
    double nb=evaluate_non_bonded(moved);
    IMP_LOG(TERSE, "New energy is " << ret << " + " << nb << std::endl);
    return ret+nb;
  }
}

double MonteCarlo::evaluate_incremental_if_below(const ParticleIndexes &,
                                     double ) const {
  IMP_NOT_IMPLEMENTED;
}


void MonteCarlo::set_close_pair_score(PairScore *ps,
                                      double distance,
                                      const ParticlesTemp &particles,
                                      const PairFilters &filters) {
  nbl_= NBLScore(ps, distance, particles, filters);
  dnn_= new algebra::DynamicNearestNeighbor3D
    (algebra::Vector3Ds(particles.size(),
                        algebra::get_zero_vector_d<3>()),
     distance);
  to_dnn_.resize(nbl_.cache_.size(), -1);
  from_dnn_.resize(particles.size());
  for (unsigned int i=0; i< nbl_.pis_.size(); ++i) {
    to_dnn_[nbl_.pis_[i]]=i;
    from_dnn_[i]=nbl_.pis_[i];
  }
}


namespace {
struct LessIndex {
  bool operator()(const std::pair<ParticleIndex, double> &dp,
                  ParticleIndex pi) {
    return dp.first < pi;
  }
  bool operator()(ParticleIndex pi,
                  const std::pair<ParticleIndex, double> &dp) {
    return pi < dp.first;
  }
  bool operator()(const std::pair<ParticleIndex, double> &dp,
                  const std::pair<ParticleIndex, double> &dq) {
    return dp.first < dq.first;
  }
};
}

MonteCarlo::NBLScore::NBLScore(PairScore *score,
                   double distance,
                   const ParticlesTemp &ps,
                   const PairFilters &filters) {
  score_=score;
  distance_=distance;
  ParticleIndex mi=0;
  pis_.resize(ps.size());
  filters_=filters;
  for (unsigned int i=0; i< ps.size(); ++i) {
    mi=std::max(mi, ps[i]->get_index());
    pis_[i]=ps[i]->get_index();
  }
  cache_.resize(mi+1);
}
double MonteCarlo::NBLScore::get_score(Model *m, ParticleIndex moved,
                                       const ParticleIndexes& nearby) const {
  IMP_LOG(VERBOSE, "Moving " << moved << std::endl);
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    double nscore=0;
    for (unsigned int i=0; i< cache_.size(); ++i) {
      for (unsigned int j=0; j< cache_[i].size(); ++j) {
        if (static_cast<unsigned int>(cache_[i][j].first) < i) {
          nscore+=cache_[i][j].second;
        }
      }
    }
    IMP_INTERNAL_CHECK(std::abs(nscore-prior_) < .05*(nscore+prior_)+.1,
                       "Cached and stored scores don't match: " << prior_
                       << " vs " << nscore);
  }
  if (moved==-1) return prior_;
  double old=0;
  removed_.clear();

  // clean out old pairs
  for (unsigned int i=0; i< cache_[moved].size(); ++i) {
    double cur=cache_[moved][i].second;
    old+=cache_[moved][i].second;
    ParticleIndex opi= cache_[moved][i].first;
    for (unsigned int j=0; j< cache_[opi].size(); ++j) {
      if (cache_[opi][j].first==moved) {
        cache_[opi].erase(cache_[opi].begin()+j);
        break;
      }
    }
    removed_.push_back(std::make_pair(ParticleIndexPair(moved, opi),cur));
  }
  added_.clear();
  cache_[moved].clear();
  ParticleIndexPairs all(nearby.size());
  for (unsigned int i=0; i< all.size(); ++i) {
    all[i]= ParticleIndexPair(moved, nearby[i]);
  }
  for (unsigned int i=0; i< filters_.size(); ++i) {
    filters_[i]->filter_in_place(m, all);
  }
  double nscore=0;
  cache_[moved].resize(nearby.size());
  for (unsigned int i=0; i< all.size(); ++i) {
    double cur= score_->evaluate_index(m, ParticleIndexPair(moved, all[i][1]),
                                       nullptr);
    if (cur!=0) {
      add_pair(moved, all[i][1], cur);
      add_pair(all[i][1], moved, cur);
      added_.push_back(all[i][1]);
      nscore+=cur;
    }
  }
  double ret= nscore-old+prior_;
  old_prior_=prior_;
  prior_=ret;
  return ret;
}

void MonteCarlo::NBLScore::roll_back(Model *, ParticleIndex moved) {
  IMP_LOG(VERBOSE, "Rolling back nbl on " << moved << std::endl);
  prior_=old_prior_;
  cache_[moved].clear();
  for (unsigned int i=0; i< added_.size(); ++i) {
    cache_[added_[i]].pop_back();
  }
  for (unsigned int i=0; i< removed_.size(); ++i) {
    add_pair(removed_[i].first[0], removed_[i].first[1], removed_[i].second);
    add_pair(removed_[i].first[1], removed_[i].first[0], removed_[i].second);
  }
}


void MonteCarlo::NBLScore::add_pair(ParticleIndex a, ParticleIndex b,
                                    double s) const {
  cache_[a].push_back(ScorePair(b,s));
}

void MonteCarlo::NBLScore::initialize(Model *m,  ParticleIndexPairs all) {
  for (unsigned int i=0; i< filters_.size(); ++i) {
    filters_[i]->filter_in_place(m, all);
  }
  for (unsigned int i=0; i< cache_.size(); ++i) {
    cache_[i].clear();
  }
  prior_=0;
  for (unsigned int i=0; i< all.size(); ++i) {
    double cur= score_->evaluate_index(m, all[i], nullptr);
    if (cur != 0) {
      prior_+=cur;
      add_pair(all[i][0], all[i][1], cur);
      add_pair(all[i][1], all[i][0], cur);
    }
  }
}

double MonteCarlo::evaluate_non_bonded(const ParticleIndexes &moved) const {
  if (!dnn_) return 0;
  if (moved.empty()) {
    return nbl_.get_score(get_model(), -1, ParticleIndexes());
  } else {
    IMP_USAGE_CHECK(moved.size()==1, "Only one at a time supported");
    Ints nearby= dnn_->get_in_ball(to_dnn_[moved[0]],
                                   nbl_.distance_);
    ParticleIndexes pisnearby(nearby.size());
    for (unsigned int i=0; i< pisnearby.size(); ++i) {
      pisnearby[i]=from_dnn_[nearby[i]];
    }
    moved_=moved[0];
    return nbl_.get_score(get_model(), moved[0], pisnearby);
  }
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
