/**
 *  \file ClassnameContainer.cpp   \brief Container for classname.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/classname_macros.h"
#include "IMP/kernel/ClassnameContainer.h"
#include "IMP/kernel/ClassnameModifier.h"
#include "IMP/kernel/internal/container_helpers.h"
#include "IMP/kernel/internal/DynamicListContainer.h"
#include "IMP/kernel/internal/StaticListContainer.h"
#include "IMP/kernel/internal/utility.h"
#include "IMP/kernel/ModelObject.h"
#include <algorithm>
#include <IMP/base/check_macros.h>
#include <boost/unordered_set.hpp>
#include <IMP/kernel/classname_macros.h>
#include <IMP/kernel/ClassnameModifier.h>
#include <IMP/kernel/ClassnamePredicate.h>
#include <IMP/kernel/ClassnameScore.h>
#include <IMP/kernel/functor.h>
#include <IMP/kernel/internal/TupleRestraint.h>
#include <IMP/kernel/internal/utility.h>
#include <IMP/kernel/Restraint.h>

IMPKERNEL_BEGIN_NAMESPACE

ClassnameContainer::ClassnameContainer(kernel::Model *m, std::string name)
: Container(m, name), contents_hash_(-1), cache_initialized_(false) {}

// here for gcc
ClassnameContainer::~ClassnameContainer() {}

PLURALVARIABLETYPE ClassnameContainer::get_FUNCTIONNAMEs() const {
  return IMP::kernel::internal::get_particle(get_model(), get_indexes());
}

bool ClassnameContainer::get_contains_FUNCTIONNAME(VARIABLETYPE v) const {
  IMPKERNEL_DEPRECATED_METHOD_DEF(2.1,
                                  "You should build your own index instead");
  INDEXTYPE iv = IMP::kernel::internal::get_index(v);
  IMP_FOREACH(INDEXTYPE it, get_contents()) {
    if (it == iv)
      return true;
  }
  return false;
}

unsigned int ClassnameContainer::get_number_of_FUNCTIONNAMEs() const {
  IMPKERNEL_DEPRECATED_METHOD_DEF(2.1, "Use get_indexes()");
  return get_number();
}

VARIABLETYPE ClassnameContainer::get_FUNCTIONNAME(unsigned int i) const {
  IMPKERNEL_DEPRECATED_METHOD_DEF(2.1, "Use get_indexes()");
  return get(i);
}

bool ClassnameContainer::get_provides_access() const {
  validate_readable();
  return do_get_provides_access();
}

void ClassnameContainer::apply_generic(const ClassnameModifier *m) const {
  apply(m);
}

void ClassnameContainer::apply(const ClassnameModifier *sm) const {
  validate_readable();
  do_apply(sm);
}

ClassnameContainerAdaptor::ClassnameContainerAdaptor(ClassnameContainer *c)
    : P(c) {}

ClassnameContainerAdaptor::ClassnameContainerAdaptor(
    const PLURALVARIABLETYPE &t) {
  IMP_USAGE_CHECK(t.size() > 0,
                  "An Empty PLURALVARIABLETYPE list cannot be adapted to "
                  "container since it lacks model info");
  kernel::Model *m = internal::get_model(t);
  IMP_NEW(internal::StaticListContainer<ClassnameContainer>, c,
          (m, "ClassnameContainerInput%1%"));
  c->set(IMP::kernel::internal::get_index(t));
  P::operator=(c);
}

void ClassnameContainerAdaptor::set_name_if_default(std::string name) {
  IMP_USAGE_CHECK(*this, "NULL passed as input");
  base::Object *o = *this;
  if (o->get_name().find("ClassnameContainerInput") == 0) {
    o->set_name(name);
  }
}

ClassnameModifier::ClassnameModifier(std::string name) : Object(name) {}

void ClassnameModifier::apply_index(kernel::Model *m, PASSINDEXTYPE v) const {
  apply(internal::get_particle(m, v));
}

void ClassnameModifier::apply(ARGUMENTTYPE vt) const {
  IMPKERNEL_DEPRECATED_METHOD_DEF(2.1, "Use index version");
  return apply_index(internal::get_model(vt), internal::get_index(vt));
}

ClassnamePredicate::ClassnamePredicate(std::string name) : Object(name) {
  /* Implemented here rather than in the header so that PairPredicate
     symbols are present in the kernel DSO */
}

void ClassnamePredicate::remove_if_equal(kernel::Model *m, PLURALINDEXTYPE &ps,
                                         int value) const {
  ps.erase(std::remove_if(ps.begin(), ps.end(),
                          make_predicate_equal(this, m, value)),
           ps.end());
}
Ints ClassnamePredicate::get_value(const PLURALVARIABLETYPE &o) const {
  IMPKERNEL_DEPRECATED_METHOD_DEF(2.1, "Use index version");
  if (o.empty()) return Ints();
  Ints ret(o.size());
  kernel::Model *m = internal::get_model(o[0]);
  for (unsigned int i = 0; i < o.size(); ++i) {
    ret[i] += get_value_index(m, internal::get_index(o[i]));
  }
  return ret;
}

int ClassnamePredicate::get_value_index(kernel::Model *m,
                                        PASSINDEXTYPE vt) const {
  return get_value(internal::get_particle(m, vt));
}

void ClassnamePredicate::remove_if_not_equal(kernel::Model *m,
                                             PLURALINDEXTYPE &ps,
                                             int value) const {
  ps.erase(std::remove_if(ps.begin(), ps.end(),
                          make_predicate_not_equal(this, m, value)),
           ps.end());
}

int ClassnamePredicate::get_value(ARGUMENTTYPE vt) const {
  IMPKERNEL_DEPRECATED_METHOD_DEF(2.1, "Use index version");
  return get_value_index(internal::get_model(vt), internal::get_index(vt));
}

ClassnameScore::ClassnameScore(std::string name) : Object(name) {
  /* Implemented here rather than in the header so that PairScore
     symbols are present in the kernel DSO */
}

double ClassnameScore::evaluate(ARGUMENTTYPE vt,
                                DerivativeAccumulator *da) const {
  IMPKERNEL_DEPRECATED_METHOD_DEF(2.1, "Use index-based evaluation instead");
  return evaluate_index(internal::get_model(vt), internal::get_index(vt), da);
}

// old versions of gcc don't like having the pragma inside the function
double ClassnameScore::evaluate_index(kernel::Model *m, PASSINDEXTYPE vt,
                                      DerivativeAccumulator *da) const {
  // see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53469
  return evaluate(internal::get_particle(m, vt), da);
}

double ClassnameScore::evaluate_indexes(kernel::Model *m,
                                        const PLURALINDEXTYPE &o,
                                        DerivativeAccumulator *da,
                                        unsigned int lower_bound,
                                        unsigned int upper_bound) const {
  double ret = 0;
  for (unsigned int i = lower_bound; i < upper_bound; ++i) {
    ret += evaluate_index(m, o[i], da);
  }
  return ret;
}

double ClassnameScore::evaluate_if_good_index(kernel::Model *m,
                                              PASSINDEXTYPE vt,
                                              DerivativeAccumulator *da,
                                              double max) const {
  IMP_UNUSED(max);
  return evaluate_index(m, vt, da);
}

double ClassnameScore::evaluate_if_good_indexes(
    kernel::Model *m, const PLURALINDEXTYPE &o, DerivativeAccumulator *da,
    double max, unsigned int lower_bound, unsigned int upper_bound) const {
  double ret = 0;
  for (unsigned int i = lower_bound; i < upper_bound; ++i) {
    double cur = evaluate_if_good_index(m, o[i], da, max - ret);
    max -= cur;
    ret += cur;
    if (max < 0) break;
  }
  return ret;
}

Restraints ClassnameScore::do_create_current_decomposition(
    kernel::Model *m, PASSINDEXTYPE vt) const {
  double score = evaluate_index(m, vt, nullptr);
  if (score == 0) {
    return Restraints();
  } else {
    return Restraints(1, IMP::kernel::internal::create_tuple_restraint(
                             this, m, vt, get_name()));
  }
}

Restraints ClassnameScore::create_current_decomposition(
    kernel::Model *m, PASSINDEXTYPE vt) const {
  return do_create_current_decomposition(m, vt);
}

IMPKERNEL_END_NAMESPACE
