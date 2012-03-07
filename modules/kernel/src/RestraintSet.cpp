/**
 *  \file RestraintSet.cpp   \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */


#include <IMP/RestraintSet.h>
#include <IMP/Model.h>
#include <IMP/log.h>
#include <IMP/internal/utility.h>
#include <IMP/generic.h>
#include <IMP/ScoringFunction.h>
#include <boost/tuple/tuple.hpp>
#include <memory>
#include <utility>
#include <numeric>

IMP_BEGIN_NAMESPACE

RestraintSet::RestraintSet(Model *m, double weight,
                           const std::string& name)
    : Restraint(m, name)
{
  set_weight(weight);
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


IMP_LIST_IMPL(RestraintSet, Restraint, restraint, Restraint*,
              Restraints);


double RestraintSet::unprotected_evaluate(DerivativeAccumulator *accum) const {
  IMP_UNUSED(accum);
  IMP_FAILURE("Special cased");
}
double RestraintSet::unprotected_evaluate_if_good(DerivativeAccumulator *accum,
                                                  double max) const {
  IMP_UNUSED(accum);
  IMP_UNUSED(max);
  IMP_FAILURE("Special cased");
}
double RestraintSet::unprotected_evaluate_if_below(DerivativeAccumulator *,
                                                   double ) const {
  IMP_FAILURE("Special cased");
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

ParticlesTemp RestraintSet::get_input_particles() const
{
  ParticlesTemp ret;
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    ret= ret+(*it)->get_input_particles();
  }
  return ret;
}

ContainersTemp RestraintSet::get_input_containers() const {
  ContainersTemp ret;
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    ret= ret+(*it)->get_input_containers();
  }
  return ret;
}


void RestraintSet::on_add(Restraint*obj) {
if (get_is_part_of_model()) {
                      obj->set_model(get_model());
                      get_model()->reset_dependencies();
                    }
                    obj->set_was_used(true);
                    IMP_USAGE_CHECK(obj != this,
                                    "Cannot add a restraint set to itself");
}
void RestraintSet::on_change() {
  if (get_is_part_of_model()) {
    get_model()->reset_dependencies();
  }
}
void RestraintSet::on_remove(RestraintSet *container, Restraint* obj) {
  if (container) obj->get_model()->reset_dependencies();
}

Restraints RestraintSet::do_create_decomposition() const {
  Restraints ret;
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    Pointer<Restraint> r=(*it)->create_decomposition();
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
    Pointer<Restraint> r=(*it)->create_current_decomposition();
    if (r) {
      ret.push_back(r);
    }
  }
  return ret;
}


Restraints
RestraintSet::do_create_incremental_decomposition(unsigned int n) const {
  Restraints ret;
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    Pointer<Restraint> r=(*it)->create_incremental_decomposition(n);
    if (r) {
      ret.push_back(r);
    }
  }
  return ret;
}


ScoringFunction* RestraintSet::create_scoring_function(double weight,
                                                      double max) const {
  return IMP::create_scoring_function(const_cast<RestraintSet*>(this), weight,
                                      max);
}

RestraintsAndWeights get_restraints_and_weights(const RestraintsTemp &rs,
                                    double initial_weight) {
  return get_restraints_and_weights(rs.begin(), rs.end(), initial_weight);
}


RestraintsTemp get_restraints(const RestraintsTemp &rs) {
  return get_restraints(rs.begin(), rs.end());
}


namespace {
  unsigned int num_children(Restraint*r) {
    RestraintSet *rs= dynamic_cast<RestraintSet*>(r);
    if (rs) return rs->get_number_of_restraints();
    else return 0;
  }
}

void show_restraint_hierarchy(Restraint *r, std::ostream &out) {
  RestraintSet *rs=dynamic_cast<RestraintSet*>(r);
  if (!rs) {
    IMP_PRINT_TREE(out, Restraint*, r, 0,
                   dynamic_cast<RestraintSet*>(n)->get_restraint,
                   out << Showable(n) << " " << n->get_maximum_score() << " "
                   << n->get_weight() );
  } else {
    IMP_PRINT_TREE(out, Restraint*, rs, num_children(n),
                   dynamic_cast<RestraintSet*>(n)->get_restraint,
                   out << Showable(n) << " " << n->get_maximum_score() << " "
                   << n->get_weight() );
  }
}


IMP_END_NAMESPACE
