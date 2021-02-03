/**
 *  \file ClassnameContainerStatistics.cpp   \brief Container for classname.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/algebra/internal/MinimalSet.h"
#include "IMP/ClassnameModifier.h"
#include "IMP/ClassnameModifier.h"
#include "IMP/ClassnameScore.h"
#include "IMP/ClassnameScore.h"
#include "IMP/container/ClassnameContainerSet.h"
#include "IMP/container/ClassnameContainerStatistics.h"
#include "IMP/container/ClassnamesOptimizerState.h"
#include "IMP/container/DistributeClassnamesScoreState.h"
#include "IMP/container/DynamicListClassnameContainer.h"
#include "IMP/container/InContainerClassnameFilter.h"
#include "IMP/container/internal/ClassnameContainerIndex.h"
#include "IMP/container/ListClassnameContainer.h"
#include "IMP/container/MinimumClassnameRestraint.h"
#include "IMP/container/PredicateClassnamesRestraint.h"
#include <algorithm>
#include <IMP/classname_macros.h>
#include <IMP/container/EventClassnamesOptimizerState.h>
#include <IMP/container/MinimumClassnameScore.h>
#include <IMP/container_macros.h>
#include <IMP/core/ClassnameRestraint.h>
#include <IMP/internal/container_helpers.h>
#include <IMP/internal/TupleRestraint.h>
#include <boost/functional/hash/hash.hpp>
#include <IMP/Optimizer.h>
#include <limits>
#include <utility>

IMPCONTAINER_BEGIN_INTERNAL_NAMESPACE

ClassnameContainerIndex::ClassnameContainerIndex(ClassnameContainerAdaptor c,
                                                 bool handle_permutations)
    : ScoreState(c->get_model(), c->get_name() + " index"),
      container_(c),
      container_version_(c->get_contents_hash()),
      handle_permutations_(handle_permutations) {
  build();
}

void ClassnameContainerIndex::build() {
  contents_.clear();
  IMP_FOREACH(INDEXTYPE it, container_->get_contents()) {
    contents_.insert(IMP::internal::get_canonical(it));
  }
}

void ClassnameContainerIndex::do_before_evaluate() {
  std::size_t h = container_->get_contents_hash();
  if (h != container_version_) {
    container_version_ = h;
    build();
  }
}

void ClassnameContainerIndex::do_after_evaluate(DerivativeAccumulator *) {}
ModelObjectsTemp ClassnameContainerIndex::do_get_inputs() const {
  return ModelObjectsTemp(1, container_);
}
ModelObjectsTemp ClassnameContainerIndex::do_get_outputs() const {
  return ModelObjectsTemp();
}

IMPCONTAINER_END_INTERNAL_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE

ClassnameContainerSet::ClassnameContainerSet(Model *m, std::string name)
    : ClassnameContainer(m, name) {}

ClassnameContainerSet::ClassnameContainerSet(const ClassnameContainersTemp &in,
                                             std::string name)
    : ClassnameContainer(IMP::internal::get_model(in), name) {
  set_CLASSFUNCTIONNAME_containers(in);
}

PLURALINDEXTYPE ClassnameContainerSet::get_indexes() const {
  PLURALINDEXTYPE sum;
  IMP_FOREACH(ClassnameContainer * c, get_classname_containers()) {
    PLURALINDEXTYPE const& cur= c->get_contents();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

//! This may be faster than get_indexes() since memory doesn't have to
//! be reallocated
void
ClassnameContainerSet::get_indexes_in_place
(PLURALINDEXTYPE& output) const {
  output.clear();
  IMP_FOREACH(ClassnameContainer * c, get_classname_containers()) {
    PLURALINDEXTYPE const& cur = c->get_contents();
    output.insert(output.end(), cur.begin(), cur.end());
  }
}

PLURALINDEXTYPE ClassnameContainerSet::get_range_indexes() const {
  PLURALINDEXTYPE sum;
  IMP_FOREACH(ClassnameContainer * c, get_classname_containers()) {
    PLURALINDEXTYPE cur = c->get_range_indexes();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

IMP_LIST_IMPL(ClassnameContainerSet, ClassnameContainer,
              CLASSFUNCTIONNAME_container, ClassnameContainer *,
              ClassnameContainers);

void ClassnameContainerSet::do_apply(const ClassnameModifier *sm) const {
  IMP_FOREACH(ClassnameContainer * c, get_classname_containers()) {
    c->apply(sm);
  }
}

ParticleIndexes ClassnameContainerSet::get_all_possible_indexes() const {
  ParticleIndexes ret;
  IMP_FOREACH(ClassnameContainer * c, get_classname_containers()) {
    ret += c->get_all_possible_indexes();
  }
  return ret;
}

ModelObjectsTemp ClassnameContainerSet::do_get_inputs() const {
  return ModelObjectsTemp(CLASSFUNCTIONNAME_containers_begin(),
                                  CLASSFUNCTIONNAME_containers_end());
}

std::size_t ClassnameContainerSet::do_get_contents_hash() const {
  std::size_t ret = 0;
  IMP_FOREACH(ClassnameContainer * c, get_classname_containers()) {
    boost::hash_combine(ret, c->get_contents_hash());
  }
  return ret;
}

IMPCONTAINER_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE

ClassnameContainerStatistics::ClassnameContainerStatistics(
    ClassnameContainerAdaptor c)
    : ScoreState(c->get_model(), c->get_name() + " statistics") {
  container_ = c;
  total_ = 0;
  max_ = 0;
  min_ = std::numeric_limits<unsigned int>::max();
  checks_ = 0;
  track_unique_ = false;
}

void ClassnameContainerStatistics::set_track_unique(bool tf) {
  track_unique_ = tf;
  unique_.clear();
}
void ClassnameContainerStatistics::show_statistics(std::ostream &out) const {
  out << "Average: " << static_cast<double>(total_) / checks_ << std::endl;
  out << "Min, Max: " << min_ << ", " << max_ << std::endl;
  if (track_unique_) {
    out << "Number unique: " << unique_.size() << std::endl;
  }
}

void ClassnameContainerStatistics::do_before_evaluate() {
  unsigned int n = container_->get_number();
  total_ += n;
  ++checks_;
  max_ = std::max(max_, n);
  min_ = std::min(min_, n);
  if (track_unique_) {
    for (unsigned int i = 0; i < container_->get_number(); ++i) {
      unique_.insert(container_->get(i));
    }
  }
}

void ClassnameContainerStatistics::do_after_evaluate(DerivativeAccumulator *) {}

ModelObjectsTemp ClassnameContainerStatistics::do_get_inputs() const {
  return ModelObjectsTemp(1, container_);
}
ModelObjectsTemp ClassnameContainerStatistics::do_get_outputs() const {
  return ModelObjectsTemp();
}

IMPCONTAINER_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE

ClassnamesOptimizerState::ClassnamesOptimizerState(ClassnameContainerAdaptor c,
                                                   ClassnameModifier *gm,
                                                   std::string name)
    : OptimizerState(c->get_model(), name), c_(c) {
  f_ = gm;
}

void ClassnamesOptimizerState::update() {
  IMP_OBJECT_LOG;
  if (!f_) return;
  IMP_LOG_TERSE("Begin ClassnamesOptimizerState::update" << std::endl);
  IMP_CHECK_OBJECT(f_);
  IMP_CHECK_OBJECT(c_);
  c_->apply(f_);

  IMP_LOG_TERSE("End ClassnamesOptimizerState::update" << std::endl);
}

IMPCONTAINER_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE
DistributeClassnamesScoreState::DistributeClassnamesScoreState(
    ClassnameContainerAdaptor input, std::string name)
    : ScoreState(input->get_model(), name) {
  input_ = input;
  input_version_ = -1;
}

ModelObjectsTemp DistributeClassnamesScoreState::do_get_outputs() const {
  ModelObjectsTemp ret;
  for (unsigned int i = 0; i < data_.size(); ++i) {
    ret.push_back(data_[i].get<0>());
  }
  return ret;
}

ModelObjectsTemp DistributeClassnamesScoreState::do_get_inputs() const {
  ModelObjectsTemp ret;
  ParticleIndexes pis = input_->get_all_possible_indexes();
  for (unsigned int i = 0; i < data_.size(); ++i) {
    ret += data_[i].get<1>()->get_inputs(get_model(), pis);
  }
  ret.push_back(input_);
  return ret;
}

void DistributeClassnamesScoreState::do_before_evaluate() {
  update_lists_if_necessary();
}
void DistributeClassnamesScoreState::do_after_evaluate(
    DerivativeAccumulator *) {}

void DistributeClassnamesScoreState::update_lists_if_necessary() const {
  std::size_t h = input_->get_contents_hash();
  if (h == input_version_) return;
  input_version_ = h;

  Vector<PLURALINDEXTYPE> output(data_.size());
  IMP_FOREACH(INDEXTYPE it, input_->get_contents()) {
    for (unsigned int i = 0; i < data_.size(); ++i) {
      if (data_[i].get<1>()->get_value_index(get_model(), it) ==
          data_[i].get<2>()) {
        output[i].push_back(it);
      }
    }
  }
  for (unsigned int i = 0; i < output.size(); ++i) {
    data_[i].get<0>()->set(output[i]);
  }
}

IMPCONTAINER_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE

DynamicListClassnameContainer::DynamicListClassnameContainer(Container *m,
                                                             std::string name)
    : P(m, name) {}

IMPCONTAINER_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE
EventClassnamesOptimizerState::EventClassnamesOptimizerState(
    ClassnamePredicate *pred, ClassnameContainerAdaptor container, int value,
    int min_count, int max_count, std::string name)
    : OptimizerState(container->get_model(), name),
      pred_(pred),
      container_(container),
      v_(value),
      min_(min_count),
      max_(max_count) {}

void EventClassnamesOptimizerState::update() {
  int met = 0;
  Model *m = get_optimizer()->get_model();
  IMP_FOREACH(INDEXTYPE it, container_->get_contents()) {
    if (pred_->get_value_index(m, it) == v_) ++met;
  }
  if (met >= min_ && met < max_) {
    throw IMP::EventException("an event occurred");
  }
}
IMPCONTAINER_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE

InContainerClassnameFilter::InContainerClassnameFilter(ClassnameContainer *c,
                                                       std::string name)
    : ClassnamePredicate(name) {
  c_ = new internal::ClassnameContainerIndex(c, true);
}

InContainerClassnameFilter::InContainerClassnameFilter(ClassnameContainer *c,
                                                       bool handle_permutations,
                                                       std::string name)
    : ClassnamePredicate(name) {
  c_ = new internal::ClassnameContainerIndex(c, handle_permutations);
}

IMPCONTAINER_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE

ListClassnameContainer::ListClassnameContainer(Model *m,
                                               const PLURALINDEXTYPE &ps,
                                               std::string name)
    : P(m, name) {
  set(ps);
}

ListClassnameContainer::ListClassnameContainer(Model *m,
                                               std::string name)
    : P(m, name) {}

ListClassnameContainer::ListClassnameContainer(Model *m,
                                               const char *name)
    : P(m, name) {}

IMPCONTAINER_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE

MinimumClassnameRestraint::MinimumClassnameRestraint(
    ClassnameScore *f, ClassnameContainerAdaptor c, unsigned int n,
    std::string name)
    : Restraint(c->get_model(), name), f_(f), c_(c), n_(n) {}

namespace {
typedef algebra::internal::MinimalSet<double, INDEXTYPE, std::less<double> >
    ClassnameMinimumMS;
template <class C, class F>
ClassnameMinimumMS find_minimal_set_ClassnameMinimum(C *c, F *f,
                                                     unsigned int n) {
  IMP_LOG_VERBOSE("Finding Minimum " << n << " of " << c->get_number()
                                     << std::endl);
  ClassnameMinimumMS bestn(n);
  IMP_FOREACH(INDEXTYPE it, c->get_contents()) {
    double score = f->evaluate_index(c->get_model(), it, nullptr);
    IMP_LOG_VERBOSE("Found " << score << " for " << it << std::endl);
    bestn.insert(score, it);
  }
  return bestn;
}
}

double MinimumClassnameRestraint::unprotected_evaluate(
    DerivativeAccumulator *da) const {
  IMP_OBJECT_LOG;
  ClassnameMinimumMS bestn =
      find_minimal_set_ClassnameMinimum(c_.get(), f_.get(), n_);

  double score = 0;
  for (unsigned int i = 0; i < bestn.size(); ++i) {
    if (da) {
      f_->evaluate_index(get_model(), bestn[i].second, da);
    }
    score += bestn[i].first;
  }
  IMP_LOG_VERBOSE("Total score is " << score << std::endl);
  return score;
}

double MinimumClassnameRestraint::unprotected_evaluate_if_good(
    DerivativeAccumulator *da, double max) const {
  IMP_OBJECT_LOG;
  ClassnameMinimumMS bestn =
      find_minimal_set_ClassnameMinimum(c_.get(), f_.get(), n_);

  double score = 0;
  for (unsigned int i = 0; i < bestn.size(); ++i) {
    if (da) {
      f_->evaluate_index(get_model(), bestn[i].second, da);
    }
    score += bestn[i].first;
    if (score > max) break;
  }
  IMP_LOG_VERBOSE("Total score is " << score << std::endl);
  return score;
}

Restraints MinimumClassnameRestraint::do_create_current_decomposition() const {
  IMP_OBJECT_LOG;
  ClassnameMinimumMS bestn =
      find_minimal_set_ClassnameMinimum(c_.get(), f_.get(), n_);
  Restraints ret;
  for (unsigned int i = 0; i < bestn.size(); ++i) {
    ret.push_back(IMP::internal::create_tuple_restraint(
        f_.get(), get_model(), bestn[i].second, get_name()));
    ret.back()->set_last_score(bestn[i].first);
  }
  return ret;
}

ModelObjectsTemp MinimumClassnameRestraint::do_get_inputs() const {
  ModelObjectsTemp ret;
  ret += f_->get_inputs(get_model(), c_->get_all_possible_indexes());
  ret.push_back(c_);
  return ret;
}

IMPCONTAINER_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE

MinimumClassnameScore::MinimumClassnameScore(const ClassnameScoresTemp &scores,
                                             unsigned int n, std::string name)
    : ClassnameScore(name), scores_(scores.begin(), scores.end()), n_(n) {}

namespace {
typedef algebra::internal::MinimalSet<
    double, ClassnameScore *, std::less<double> > MinimumClassnameScoreMS;
template <class It>
MinimumClassnameScoreMS find_minimal_set_MinimumClassnameScore(It b, It e,
                                                               Model *m,
                                                               PASSINDEXTYPE v,
                                                               unsigned int n) {
  IMP_LOG_TERSE("Finding Minimum " << n << " of " << std::distance(b, e)
                                   << std::endl);
  MinimumClassnameScoreMS bestn(n);
  for (It it = b; it != e; ++it) {
    double score = (*it)->evaluate_index(m, v, nullptr);
    bestn.insert(score, *it);
  }
  return bestn;
}
}

double MinimumClassnameScore::evaluate_index(Model *m, PASSINDEXTYPE v,
                                             DerivativeAccumulator *da) const {
  MinimumClassnameScoreMS bestn = find_minimal_set_MinimumClassnameScore(
      scores_.begin(), scores_.end(), m, v, n_);

  double score = 0;
  for (unsigned int i = 0; i < bestn.size(); ++i) {
    if (da) {
      bestn[i].second->evaluate_index(m, v, da);
    }
    score += bestn[i].first;
  }
  return score;
}

ModelObjectsTemp MinimumClassnameScore::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret;
  for (unsigned int i = 0; i < scores_.size(); ++i) {
    ret += scores_[i]->get_inputs(m, pis);
  }
  return ret;
}

Restraints MinimumClassnameScore::do_create_current_decomposition(
    Model *m, PASSINDEXTYPE vt) const {
  Restraints ret;
  MinimumClassnameScoreMS bestn = find_minimal_set_MinimumClassnameScore(
      scores_.begin(), scores_.end(), m, vt, n_);
  for (unsigned int i = 0; i < bestn.size(); ++i) {
    ret.push_back(
        IMP::internal::create_tuple_restraint(bestn[i].second, m, vt));
    ret.back()->set_last_score(bestn[i].first);
  }
  return ret;
}

IMPCONTAINER_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE

PredicateClassnamesRestraint::PredicateClassnamesRestraint(
    ClassnamePredicate *pred, ClassnameContainerAdaptor input, std::string name)
    : Restraint(input->get_model(), name),
      predicate_(pred),
      input_(input),
      is_get_inputs_ignores_individual_scores_(false),
      input_version_(input->get_contents_hash()),
      //      is_unknown_score_set_(false),
      error_on_unknown_(true),
      unknown_score_(nullptr)
{
#ifdef IMP_CONTAINER_PREDICATE_USE_GOOGLE_DENSE_HASH_MAP
  lists_.set_empty_key(-1);
  scores_.set_empty_key(-1);
#endif
}

void PredicateClassnamesRestraint::do_add_score_and_derivatives(
    ScoreAccumulator sa) const {
  // currently ignores all maxima
  // no longer parallizable
  update_lists_if_necessary();
  typedef std::pair<int, PLURALINDEXTYPE> LP;
  IMP_FOREACH(const LP & lp, lists_) {
    IMP_LOG_VERBOSE("Evaluating score for predicate value " << lp.first
                                                            << std::endl);
    ClassnameScore* score= get_score_for_predicate(lp.first);
    if(IMP_LIKELY(score != nullptr)){
      double cur_score = score->evaluate_indexes(get_model(), lp.second,
                                                 sa.get_derivative_accumulator(),
                                                 0, lp.second.size());
      sa.add_score(cur_score);
    }
  }
}

ModelObjectsTemp PredicateClassnamesRestraint::do_get_inputs() const {
  ModelObjectsTemp ret;
  ret.push_back(input_);
  ParticleIndexes all = input_->get_all_possible_indexes();
  ret += predicate_->get_inputs(get_model(), all);
  if(!is_get_inputs_ignores_individual_scores_){
    typedef std::pair<int, PointerMember<ClassnameScore> > SP;
    IMP_FOREACH(const SP & sp, scores_) {
      ret += sp.second->get_inputs(get_model(), all);
    }
  }
  return ret;
}

Restraints PredicateClassnamesRestraint::do_create_current_decomposition()
    const {
  Restraints ret;
  typedef std::pair<int, PLURALINDEXTYPE> LP;
  IMP_FOREACH(const LP & lp, lists_) {
    if(lists_.size()>0){
      ClassnameScore* score= get_score_for_predicate(lp.first);
      if(IMP_LIKELY(score != nullptr)){
        IMP_FOREACH(PASSINDEXTYPE it, lp.second) {
          Restraints r =
            score->create_current_decomposition(get_model(), it);
          ret += r;
        }
      }
    }
  }
  return ret;
}

//! populate lists with bins of PLURALINDEXTYPE for each predicate,
//! and put unknown predicates in unknown_bin, if unknown score exists
void PredicateClassnamesRestraint::update_lists_if_necessary() const {
  // Compare hash to prevent needless refresh:
  std::size_t h = input_->get_contents_hash();
  if (IMP_UNLIKELY(h == input_version_)){
    return;
  }
  input_version_ = h;
#ifdef IMP_CONTAINER_PREDICATE_USE_ROBIN_MAP
  for(t_lists_map::iterator lists_it= lists_.begin();
      lists_it != lists_.end(); lists_it++){
    lists_it.value().clear();
  }
#else
  typedef std::pair<const int, PLURALINDEXTYPE> LP;
  IMP_FOREACH(LP & lp, lists_) {
    lp.second.clear();
  }
#endif
  predicate_->setup_for_get_value_index_in_batch(get_model());
  IMP_FOREACH(INDEXTYPE it, input_->get_contents()) {
    int bin = predicate_->get_value_index_in_batch(get_model(), it);
    lists_[bin].push_back(it);
  } // IMP_FOREACH
}


void PredicateClassnamesRestraint::set_score(int predicate_value,
                                             ClassnameScore *score) {
  IMP_USAGE_CHECK(predicate_value != std::numeric_limits<int>::max(),
                  "The predicate value of " << std::numeric_limits<int>::max()
                                            << " is reserved.");
  scores_[predicate_value] = score;
  score->set_was_used(true);
}

void PredicateClassnamesRestraint::set_unknown_score(ClassnameScore *score) {
  //static const int unknown_bin =
  //  std::numeric_limits<int>::max();
  error_on_unknown_ = false;
  //scores_[unknown_bin] = score;
  unknown_score_= score;
  //score->set_was_used(true);
  //is_unknown_score_set_=true;
}

IMPCONTAINER_END_NAMESPACE
