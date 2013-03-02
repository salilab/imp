/**
 *  \file RestraintSet.cpp   \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */


#include <IMP/kernel/RestraintSet.h>
#include <IMP/kernel/Model.h>
#include <IMP/kernel/log.h>
#include <IMP/kernel/internal/utility.h>
#include <IMP/kernel/generic.h>
#include <IMP/kernel/ScoringFunction.h>
#include <IMP/kernel/internal/restraint_evaluation.h>
#include <boost/tuple/tuple.hpp>
#include <memory>
#include <utility>
#include <numeric>

IMPKERNEL_BEGIN_NAMESPACE

RestraintSet::RestraintSet(Model *m, double weight,
                           const std::string& name)
    : Restraint(m, name)
{
  set_weight(weight);
}

RestraintSet::RestraintSet(const RestraintsTemp &rs, double weight,
                           const std::string& name)
    : Restraint(internal::get_model(rs), name)
{
  set_weight(weight);
  set_restraints(rs);
}


RestraintSet::RestraintSet(double weight,
                           const std::string& name)
  : Restraint(name)
{
  set_weight(weight);
}

RestraintSet::RestraintSet(const std::string& name)
  : Restraint(name)
{
}

RestraintSet::RestraintSet(ModelInitTag, const std::string& name)
    : Restraint(ModelInitTag(), name)
{
}


IMP_LIST_IMPL(RestraintSet, Restraint, restraint, Restraint*,
              Restraints);

double RestraintSet::unprotected_evaluate(DerivativeAccumulator *da) const {
  IMP_ALWAYS_CHECK(!da, "Can't do unprotected evaluation of restraint sets"
                   << " with derivatives", ValueException);
  double ret=0;
  for (unsigned int i=0; i< get_number_of_restraints(); ++i) {
    ret+=get_restraint(i)->unprotected_evaluate(nullptr);
  }
  return ret;
}

void RestraintSet::do_add_score_and_derivatives(ScoreAccumulator sa) const {
  for (unsigned int i=0; i< get_number_of_restraints(); ++i) {
    get_restraint(i)->add_score_and_derivatives(sa);
  }
  // for child tasks
IMP_OMP_PRAGMA(taskwait)
}

double
RestraintSet
::get_last_score() const {
  double ret=0;
  for (unsigned int i=0; i< get_number_of_restraints(); ++i) {
    ret+=get_restraint(i)->get_last_score();
  }
  return ret;
}

std::pair<RestraintsTemp, RestraintSetsTemp>
RestraintSet::get_non_sets_and_sets() const {
  std::pair<RestraintsTemp, RestraintSetsTemp> ret;
  for (unsigned int i=0; i< get_number_of_restraints(); ++i) {
    if (dynamic_cast<RestraintSet*>(get_restraint(i))) {
      ret.second.push_back(dynamic_cast<RestraintSet*>(get_restraint(i)));
    } else {
      ret.first.push_back(get_restraint(i));
    }
  }
  return ret;
}


void RestraintSet::set_model(Model *m) {
  Restraint::set_model(m);
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    (*it)->set_model(m);
  }
}

void RestraintSet::show_it(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    (*it)->show(out);
  }
  out << "... end restraint set " << get_name() << std::endl;
}

void RestraintSet::on_add(Restraint*obj) {
if (get_is_part_of_model()) {
                      obj->set_model(get_model());
                      get_model()->clear_caches();
                    }
                    obj->set_was_used(true);
                    IMP_USAGE_CHECK(obj != this,
                                    "Cannot add a restraint set to itself");
}
void RestraintSet::on_change() {
  if (get_is_part_of_model()) {
    get_model()->clear_caches();
  }
}

ModelObjectsTemp RestraintSet::do_get_inputs() const {
  return ModelObjectsTemp(restraints_begin(), restraints_end());
}
void RestraintSet::on_remove(RestraintSet *container, Restraint* obj) {
  if (container) obj->get_model()->clear_caches();
}

Restraints RestraintSet::do_create_decomposition() const {
  Restraints ret;
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    base::Pointer<Restraint> r=(*it)->create_decomposition();
    if (r) {
      ret.push_back(r);
    }
  }
  return ret;
}
Restraints RestraintSet::do_create_current_decomposition() const {
  Restraints ret;
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    base::Pointer<Restraint> r=(*it)->create_current_decomposition();
    if (r) {
      ret.push_back(r);
    }
  }
  return ret;
}


ScoringFunction* RestraintSet::create_scoring_function(double weight,
                                                      double max) const {
  return IMP::kernel::create_scoring_function(const_cast<RestraintSet*>(this),
                                              weight,
                                      max, get_name()+" scoring");
}

RestraintsTemp get_restraints(const RestraintsTemp &rs) {
  return get_restraints(rs.begin(), rs.end());
}


IMPKERNEL_END_NAMESPACE
