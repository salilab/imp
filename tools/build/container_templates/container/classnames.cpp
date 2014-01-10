/**
 *  \file ClassnameContainerStatistics.cpp   \brief Container for classname.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
#include <IMP/kernel/internal/container_helpers.h>
#include <IMP/kernel/internal/InternalListClassnameContainer.h>
#include <IMP/kernel/internal/TupleRestraint.h>
#include <IMP/Optimizer.h>
#include <limits>
#include <utility>

IMPCONTAINER_BEGIN_INTERNAL_NAMESPACE

ClassnameContainerIndex::ClassnameContainerIndex(ClassnameContainerAdaptor c,
                                                 bool handle_permutations)
    : ScoreState(c->get_model(), c->get_name() + " index"),
      container_(c),
      container_version_(c->get_contents_version()),
      handle_permutations_(handle_permutations) {
  build();
}

void ClassnameContainerIndex::build() {
  contents_.clear();
  IMP_CONTAINER_FOREACH(
      ClassnameContainer, container_,
      contents_.insert(IMP::kernel::internal::get_canonical(_1)));
}

void ClassnameContainerIndex::do_before_evaluate() {
  if (kernel::Container::update_version(container_, container_version_)) {
    build();
  }
}

void ClassnameContainerIndex::do_after_evaluate(DerivativeAccumulator *) {}
ModelObjectsTemp ClassnameContainerIndex::do_get_inputs() const {
  return kernel::ModelObjectsTemp(1, container_);
}
ModelObjectsTemp ClassnameContainerIndex::do_get_outputs() const {
  return kernel::ModelObjectsTemp();
}

IMPCONTAINER_END_INTERNAL_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE

ClassnameContainerSet::ClassnameContainerSet(kernel::Model *m, std::string name)
    : ClassnameContainer(m, name) {}

ClassnameContainerSet::ClassnameContainerSet(const ClassnameContainersTemp &in,
                                             std::string name)
    : ClassnameContainer(IMP::kernel::internal::get_model(in), name) {
  set_CLASSFUNCTIONNAME_containers(in);
}

PLURALINDEXTYPE ClassnameContainerSet::get_indexes() const {
  PLURALINDEXTYPE sum;
  for (ClassnameContainerConstIterator it =
           CLASSFUNCTIONNAME_containers_begin();
       it != CLASSFUNCTIONNAME_containers_end(); ++it) {
    PLURALINDEXTYPE cur = (*it)->get_indexes();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

PLURALINDEXTYPE ClassnameContainerSet::get_range_indexes() const {
  PLURALINDEXTYPE sum;
  for (ClassnameContainerConstIterator it =
           CLASSFUNCTIONNAME_containers_begin();
       it != CLASSFUNCTIONNAME_containers_end(); ++it) {
    PLURALINDEXTYPE cur = (*it)->get_range_indexes();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

IMP_LIST_IMPL(ClassnameContainerSet, ClassnameContainer,
              CLASSFUNCTIONNAME_container, ClassnameContainer *,
              ClassnameContainers);

void ClassnameContainerSet::do_apply(const ClassnameModifier *sm) const {
  for (unsigned int i = 0; i < get_number_of_CLASSFUNCTIONNAME_containers();
       ++i) {
    get_CLASSFUNCTIONNAME_container(i)->apply(sm);
  }
}

ParticleIndexes ClassnameContainerSet::get_all_possible_indexes() const {
  kernel::ParticleIndexes ret;
  for (unsigned int i = 0; i < get_number_of_CLASSFUNCTIONNAME_containers();
       ++i) {
    ret += get_CLASSFUNCTIONNAME_container(i)->get_all_possible_indexes();
  }
  return ret;
}

void ClassnameContainerSet::do_before_evaluate() {
  bool changed = false;
  versions_.resize(get_number_of_CLASSFUNCTIONNAME_containers(), -1);
  for (unsigned int i = 0; i < get_number_of_CLASSFUNCTIONNAME_containers();
       ++i) {
    changed |= update_version(get_CLASSFUNCTIONNAME_container(i), versions_[i]);
  }
  set_is_changed(changed);
}

ModelObjectsTemp ClassnameContainerSet::do_get_inputs() const {
  return kernel::ModelObjectsTemp(CLASSFUNCTIONNAME_containers_begin(),
                                  CLASSFUNCTIONNAME_containers_end());
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
  return kernel::ModelObjectsTemp(1, container_);
}
ModelObjectsTemp ClassnameContainerStatistics::do_get_outputs() const {
  return kernel::ModelObjectsTemp();
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
  kernel::ModelObjectsTemp ret;
  for (unsigned int i = 0; i < data_.size(); ++i) {
    ret.push_back(data_[i].get<0>());
  }
  return ret;
}

ModelObjectsTemp DistributeClassnamesScoreState::do_get_inputs() const {
  kernel::ModelObjectsTemp ret;
  kernel::ParticleIndexes pis = input_->get_all_possible_indexes();
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
  if (!kernel::Container::update_version(input_, input_version_)) return;

  base::Vector<PLURALINDEXTYPE> output(data_.size());
  IMP_CONTAINER_FOREACH(ClassnameContainer, input_, {
    for (unsigned int i = 0; i < data_.size(); ++i) {
      if (data_[i].get<1>()->get_value_index(get_model(), _1) ==
          data_[i].get<2>()) {
        output[i].push_back(_1);
      }
    }
  });
  for (unsigned int i = 0; i < output.size(); ++i) {
    data_[i].get<0>()->set(output[i]);
  }
}

IMPCONTAINER_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE

DynamicListClassnameContainer::DynamicListClassnameContainer(Container *m,
                                                             std::string name)
    : P(m, name) {}

void DynamicListClassnameContainer::add_FUNCTIONNAME(ARGUMENTTYPE vt) {
  add(IMP::kernel::internal::get_index(vt));
}
void DynamicListClassnameContainer::add_FUNCTIONNAMEs(
    const PLURALVARIABLETYPE &c) {
  add(IMP::kernel::internal::get_index(c));
}
void DynamicListClassnameContainer::set_FUNCTIONNAMEs(PLURALVARIABLETYPE c) {
  set(IMP::kernel::internal::get_index(c));
}
void DynamicListClassnameContainer::clear_FUNCTIONNAMEs() { clear(); }
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
  kernel::Model *m = get_optimizer()->get_model();
  IMP_CONTAINER_FOREACH(ClassnameContainer, container_,
                        if (pred_->get_value_index(m, _1) == v_) { ++met; });
  if (met >= min_ && met < max_) {
    throw IMP::base::EventException("an event occurred");
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

ListClassnameContainer::ListClassnameContainer(const PLURALVARIABLETYPE &ps,
                                               std::string name)
    : P(IMP::kernel::internal::get_model(ps[0]), name) {
  set_FUNCTIONNAMEs(ps);
}

ListClassnameContainer::ListClassnameContainer(kernel::Model *m,
                                               const PLURALINDEXTYPE &ps,
                                               std::string name)
    : P(m, name) {
  set(ps);
}

ListClassnameContainer::ListClassnameContainer(kernel::Model *m,
                                               std::string name)
    : P(m, name) {}

ListClassnameContainer::ListClassnameContainer(kernel::Model *m,
                                               const char *name)
    : P(m, name) {}

void ListClassnameContainer::add_FUNCTIONNAME(ARGUMENTTYPE vt) {
  add(IMP::kernel::internal::get_index(vt));
}
void ListClassnameContainer::add_FUNCTIONNAMEs(const PLURALVARIABLETYPE &c) {
  add(IMP::kernel::internal::get_index(c));
}
void ListClassnameContainer::set_FUNCTIONNAMEs(const PLURALVARIABLETYPE &c) {
  set(IMP::kernel::internal::get_index(c));
}
void ListClassnameContainer::set_FUNCTIONNAMEs(const PLURALINDEXTYPE &c) {
  set(c);
}
void ListClassnameContainer::clear_FUNCTIONNAMEs() { clear(); }

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
  IMP_CONTAINER_FOREACH_TEMPLATE(C, c, {
    double score = f->evaluate_index(c->get_model(), _1, nullptr);
    IMP_LOG_VERBOSE("Found " << score << " for " << _1 << std::endl);
    bestn.insert(score, _1);
  });
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
    ret.push_back(IMP::kernel::internal::create_tuple_restraint(
        f_.get(), get_model(), bestn[i].second, get_name()));
    ret.back()->set_last_score(bestn[i].first);
  }
  return ret;
}

ModelObjectsTemp MinimumClassnameRestraint::do_get_inputs() const {
  kernel::ModelObjectsTemp ret;
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
                                                               kernel::Model *m,
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

double MinimumClassnameScore::evaluate_index(kernel::Model *m, PASSINDEXTYPE v,
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
    kernel::Model *m, const kernel::ParticleIndexes &pis) const {
  kernel::ModelObjectsTemp ret;
  for (unsigned int i = 0; i < scores_.size(); ++i) {
    ret += scores_[i]->get_inputs(m, pis);
  }
  return ret;
}

Restraints MinimumClassnameScore::do_create_current_decomposition(
    kernel::Model *m, PASSINDEXTYPE vt) const {
  Restraints ret;
  MinimumClassnameScoreMS bestn = find_minimal_set_MinimumClassnameScore(
      scores_.begin(), scores_.end(), m, vt, n_);
  for (unsigned int i = 0; i < bestn.size(); ++i) {
    ret.push_back(
        IMP::kernel::internal::create_tuple_restraint(bestn[i].second, m, vt));
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
      input_version_(-1),
      error_on_unknown_(true) {}

void PredicateClassnamesRestraint::do_add_score_and_derivatives(
    ScoreAccumulator sa) const {
  // currently ignores all maxima
  // no longer parallizable
  update_lists_if_necessary();
  typedef std::pair< int, PLURALINDEXTYPE> LP;
  IMP_FOREACH(const LP & lp, lists_) {
    IMP_LOG_VERBOSE("Evaluating score for predicate value " << lp.first
                                                            << std::endl);
    kernel::ClassnameScore *score = scores_.find(lp.first)->second;
    double cur_score = score->evaluate_indexes(get_model(), lp.second,
                                               sa.get_derivative_accumulator(),
                                               0, lp.second.size());
    sa.add_score(cur_score);
  }
}

ModelObjectsTemp PredicateClassnamesRestraint::do_get_inputs() const {
  kernel::ModelObjectsTemp ret;
  kernel::ParticleIndexes all = input_->get_all_possible_indexes();
  ret += predicate_->get_inputs(get_model(), all);
  typedef std::pair<int, base::PointerMember<ClassnameScore> > SP;
  IMP_FOREACH(const SP & sp, scores_) {
    ret += sp.second->get_inputs(get_model(), all);
  }
  ret.push_back(input_);
  return ret;
}

Restraints PredicateClassnamesRestraint::do_create_current_decomposition()
    const {
  Restraints ret;
  typedef std::pair<int, PLURALINDEXTYPE> LP;
  IMP_FOREACH(const LP & lp, lists_) {
    kernel::ClassnameScore *score = scores_.find(lp.first)->second;
    IMP_FOREACH(PASSINDEXTYPE it, lp.second) {
      kernel::Restraints r =
          score->create_current_decomposition(get_model(), it);
      ret += r;
    }
  }
  return ret;
}

void PredicateClassnamesRestraint::update_lists_if_necessary() const {
  if (!kernel::Container::update_version(input_, input_version_)) return;
  lists_.clear();

  IMP_CONTAINER_FOREACH(ClassnameContainer, input_, {
    int bin = predicate_->get_value_index(get_model(), _1);
    lists_[bin].push_back(_1);
  });

  typedef std::pair<int, PLURALINDEXTYPE> LP;
  Ints unknown;
  IMP_FOREACH(const LP &lp, lists_) {
    int bin = lp.first;
    if (scores_.find(bin) == scores_.end()) {
      IMP_USAGE_CHECK(!error_on_unknown_, "Unknown predicate value of "
                                              << bin << " found for tuples "
                                              << lp.second);
      unknown.push_back(bin);
    }
  }
  const int unknown_bin = std::numeric_limits<double>::max();
  if (scores_.find(unknown_bin) != scores_.end()) {
    PLURALINDEXTYPE &unknowns = lists_.find(unknown_bin)->second;
    IMP_FOREACH(int i, unknown) { unknowns += lists_.find(i)->second; }
  }
  IMP_FOREACH(int i, unknown) { lists_.erase(i); }
}

void PredicateClassnamesRestraint::set_score(int predicate_value,
                                             kernel::ClassnameScore *score) {
  IMP_USAGE_CHECK(predicate_value != std::numeric_limits<int>::max(),
                  "The predicate value of " << std::numeric_limits<int>::max()
                                            << " is reserved.");
  scores_[predicate_value] = score;
  score->set_was_used(true);
}

void PredicateClassnamesRestraint::set_unknown_score(
    kernel::ClassnameScore *score) {
  error_on_unknown_ = false;
  scores_[std::numeric_limits<int>::max()] = score;
  score->set_was_used(true);
}

IMPCONTAINER_END_NAMESPACE
