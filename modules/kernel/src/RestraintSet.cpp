/**
 *  \file RestraintSet.cpp   \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/RestraintSet.h>
#include <IMP/Model.h>
#include <IMP/log.h>
#include <IMP/internal/utility.h>
#include <IMP/generic.h>
#include <IMP/ScoringFunction.h>
#include <IMP/internal/restraint_evaluation.h>
#include <boost/tuple/tuple.hpp>
#include <memory>
#include <utility>
#include <numeric>

IMPKERNEL_BEGIN_NAMESPACE

RestraintSet::RestraintSet(Model *m, double weight,
                           const std::string &name)
    : Restraint(m, name) {
  set_weight(weight);
  is_aggregate_ = true;
}

RestraintSet::RestraintSet(Model *m, const std::string &name)
    : Restraint(m, name) {
  set_weight(1.0);
  is_aggregate_ = true;
}

RestraintSet::RestraintSet(const RestraintsTemp &rs, double weight,
                           const std::string &name)
    : Restraint(internal::get_model(rs), name) {
  set_weight(weight);
  set_restraints(rs);
  is_aggregate_ = true;
}

IMP_LIST_IMPL(RestraintSet, Restraint, restraint, Restraint *, Restraints);

double RestraintSet::unprotected_evaluate(DerivativeAccumulator *da) const {
  IMP_ALWAYS_CHECK(!da, "Can't do unprotected evaluation of restraint sets"
                            << " with derivatives",
                   ValueException);
  double ret = 0;
  for (unsigned int i = 0; i < get_number_of_restraints(); ++i) {
    ret += get_restraint(i)->unprotected_evaluate(nullptr);
  }
  return ret;
}

void RestraintSet::do_add_score_and_derivatives(ScoreAccumulator sa) const {
  for (unsigned int i = 0; i < get_number_of_restraints(); ++i) {
    get_restraint(i)->add_score_and_derivatives(sa);
  }
  // for child tasks
  IMP_OMP_PRAGMA(taskwait)
}

namespace {
void add_last_score_restraint(Restraint *r, ScoreAccumulator &sa,
#if IMP_HAS_CHECKS >= IMP_INTERNAL
                              const ParticleIndexes &moved_pis,
                              const ParticleIndexes &reset_pis,
#endif
                              double last_score) {
  // If the restraint is new, get the full score
  if (last_score == NO_MAX) {
    r->add_score_and_derivatives(sa);
  } else {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
    r->add_score_and_derivatives_moved(sa, moved_pis, reset_pis);
    IMP_INTERNAL_CHECK_FLOAT_EQUAL(
           r->get_last_score(), last_score,
           "Restraint " << *r
           << " changed score even though particles didn't move");
#else
    sa.add_score(last_score * r->get_weight());
#endif
  }
}
}

void RestraintSet::do_add_score_and_derivatives_moved(
                ScoreAccumulator sa, const ParticleIndexes &moved_pis,
                const ParticleIndexes &reset_pis) const {
  // If we only want the score, and only a single particle moved, only
  // evaluate the restraints that depend on that particle, and use the
  // last score for the rest
  if (!sa.get_derivative_accumulator() && moved_pis.size() == 1
      && reset_pis.size() == 0) {
    const std::set<Restraint *> &rsset
           = get_model()->get_dependent_restraints(moved_pis[0]);
    for (unsigned int i = 0; i < get_number_of_restraints(); ++i) {
      Restraint *r = get_restraint(i);
      if (rsset.find(r) != rsset.end()) {
        r->add_score_and_derivatives_moved(sa, moved_pis, reset_pis);
      } else {
        add_last_score_restraint(r, sa,
#if IMP_HAS_CHECKS >= IMP_INTERNAL
                                 moved_pis, reset_pis,
#endif
                                 r->get_last_score());
      }
    }
  // If we have *both* moved and reset particles, we need to check each
  // restraint for both
  } else if (!sa.get_derivative_accumulator() && moved_pis.size() == 1
             && reset_pis.size() == 1) {
    const std::set<Restraint *> &moved_set
           = get_model()->get_dependent_restraints(moved_pis[0]);
    const std::set<Restraint *> &reset_set
           = get_model()->get_dependent_restraints(reset_pis[0]);
    for (unsigned int i = 0; i < get_number_of_restraints(); ++i) {
      Restraint *r = get_restraint(i);
      // must check moved first, since if a given restraint is affected by
      // *both* moved and reset particles, we need to recalculate it
      if (moved_set.find(r) != moved_set.end()) {
        // preserve last-last score if reset
        if (reset_set.find(r) != reset_set.end()) {
          double last_last_score = r->get_last_last_score();
          r->add_score_and_derivatives_moved(sa, moved_pis, reset_pis);
          r->set_last_last_score(last_last_score);
        } else {
          r->add_score_and_derivatives_moved(sa, moved_pis, reset_pis);
        }
      } else if (reset_set.find(r) != reset_set.end()) {
        // If reset, we can use the last-but-one score, unless it is an
        // aggregate restraint (e.g. another RestraintSet), in which case
        // have the restraint itself figure out what to return
        if (r->get_is_aggregate()) {
          r->add_score_and_derivatives_moved(sa, moved_pis, reset_pis);
        } else {
          double score = r->get_last_last_score();
          add_last_score_restraint(r, sa,
#if IMP_HAS_CHECKS >= IMP_INTERNAL
                                   moved_pis, reset_pis,
#endif
                                   score);
          r->set_last_score(score);
        }
      } else {
        // If not moved, we can use the last score
        add_last_score_restraint(r, sa,
#if IMP_HAS_CHECKS >= IMP_INTERNAL
                                 moved_pis, reset_pis,
#endif
                                 r->get_last_score());
      }
    }
  } else {
    for (unsigned int i = 0; i < get_number_of_restraints(); ++i) {
      get_restraint(i)->add_score_and_derivatives_moved(
                                    sa, moved_pis, reset_pis);
    }
  }
  // for child tasks
  IMP_OMP_PRAGMA(taskwait)
}

double RestraintSet::get_last_score() const {
  double ret = 0;
  for (unsigned int i = 0; i < get_number_of_restraints(); ++i) {
    ret += get_restraint(i)->get_last_score();
  }
  return ret;
}

std::pair<RestraintsTemp, RestraintSetsTemp>
RestraintSet::get_non_sets_and_sets() const {
  std::pair<RestraintsTemp, RestraintSetsTemp> ret;
  for (unsigned int i = 0; i < get_number_of_restraints(); ++i) {
    if (dynamic_cast<RestraintSet *>(get_restraint(i))) {
      ret.second.push_back(dynamic_cast<RestraintSet *>(get_restraint(i)));
    } else {
      ret.first.push_back(get_restraint(i));
    }
  }
  return ret;
}

void RestraintSet::show_it(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  for (RestraintConstIterator it = restraints_begin(); it != restraints_end();
       ++it) {
    (*it)->show(out);
  }
  out << "... end restraint set " << get_name() << std::endl;
}

void RestraintSet::on_add(Restraint *obj) {
  set_has_dependencies(false);
  obj->set_was_used(true);
  IMP_USAGE_CHECK(obj != this, "Cannot add a restraint set to itself");
}

void RestraintSet::on_change() {
  set_has_dependencies(false);
  // last-last score isn't just the sum of each restraint's last-last score,
  // so just invalidate it when we add/remove restraints to force recalculation
  set_last_last_score(BAD_SCORE);
}

ModelObjectsTemp RestraintSet::do_get_inputs() const {
  return ModelObjectsTemp(restraints_begin(), restraints_end());
}
void RestraintSet::on_remove(RestraintSet *container, Restraint *) {
  if (container && container->get_model()) {
    container->set_has_dependencies(false);
  }
}

Restraints RestraintSet::do_create_decomposition() const {
  Restraints ret;
  for (RestraintConstIterator it = restraints_begin(); it != restraints_end();
       ++it) {
    Pointer<Restraint> r = (*it)->create_decomposition();
    if (r) {
      ret.push_back(r);
    }
  }
  return ret;
}
Restraints RestraintSet::do_create_current_decomposition() const {
  Restraints ret;
  for (RestraintConstIterator it = restraints_begin(); it != restraints_end();
       ++it) {
    Pointer<Restraint> r = (*it)->create_current_decomposition();
    if (r) {
      ret.push_back(r);
    }
  }
  return ret;
}

ScoringFunction *RestraintSet::create_scoring_function(double weight,
                                                       double max) const {
  return IMP::create_scoring_function(
      const_cast<RestraintSet *>(this), weight, max, get_name() + " scoring");
}

RestraintsTemp get_restraints(const RestraintsTemp &rs) {
  return get_restraints(rs.begin(), rs.end());
}

IMPKERNEL_END_NAMESPACE
