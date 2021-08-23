/**
 *  \file ClassnameContainer.cpp   \brief Container for classname.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/classname_macros.h"
#include "IMP/ClassnameContainer.h"
#include "IMP/ClassnameModifier.h"
#include "IMP/internal/container_helpers.h"
#include "IMP/internal/DynamicListContainer.h"
#include "IMP/internal/StaticListContainer.h"
#include "IMP/internal/utility.h"
#include "IMP/ModelObject.h"
#include <algorithm>
#include <IMP/check_macros.h>
#include <boost/unordered_set.hpp>
#include <IMP/classname_macros.h>
#include <IMP/ClassnameModifier.h>
#include <IMP/ClassnamePredicate.h>
#include <IMP/ClassnameScore.h>
#include <IMP/functor.h>
#include <IMP/internal/TupleRestraint.h>
#include <IMP/internal/utility.h>
#include <IMP/Restraint.h>

IMPKERNEL_BEGIN_NAMESPACE

ClassnameContainer::ClassnameContainer(Model *m, std::string name)
    : Container(m, name), contents_hash_(-1), cache_initialized_(false) {}

// here for gcc
ClassnameContainer::~ClassnameContainer() {}

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
  Model *m = internal::get_model(t);
  IMP_NEW(internal::StaticListContainer<ClassnameContainer>, c,
          (m, "ClassnameContainerInput%1%"));
  c->set(IMP::internal::get_index(t));
  P::operator=(c);
}

void ClassnameContainerAdaptor::set_name_if_default(std::string name) {
  IMP_USAGE_CHECK(*this, "NULL passed as input");
  Object *o = *this;
  if (o->get_name().find("ClassnameContainerInput") == 0) {
    o->set_name(name);
  }
}

ClassnameModifier::ClassnameModifier(std::string name) : Object(name) {}

ClassnamePredicate::ClassnamePredicate(std::string name) : Object(name) {
  /* Implemented here rather than in the header so that PairPredicate
     symbols are present in the kernel DSO */
}

void ClassnamePredicate::remove_if_equal(Model *m, PLURALINDEXTYPE &ps,
                                         int value) const {
  ps.erase(std::remove_if(ps.begin(), ps.end(),
                          make_predicate_equal(this, m, value)),
           ps.end());
}

void ClassnamePredicate::remove_if_not_equal(Model *m,
                                             PLURALINDEXTYPE &ps,
                                             int value) const {
  ps.erase(std::remove_if(ps.begin(), ps.end(),
                          make_predicate_not_equal(this, m, value)),
           ps.end());
}

ClassnameScore::ClassnameScore(std::string name) : Object(name) {
  /* Implemented here rather than in the header so that PairScore
     symbols are present in the kernel DSO */
}

double ClassnameScore::evaluate_indexes(Model *m,
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

double ClassnameScore::evaluate_if_good_index(Model *m,
                                              PASSINDEXTYPE vt,
                                              DerivativeAccumulator *da,
                                              double max) const {
  IMP_UNUSED(max);
  return evaluate_index(m, vt, da);
}

double ClassnameScore::evaluate_if_good_indexes(
    Model *m, const PLURALINDEXTYPE &o, DerivativeAccumulator *da,
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
    Model *m, PASSINDEXTYPE vt) const {
  double score = evaluate_index(m, vt, nullptr);
  if (score == 0) {
    return Restraints();
  } else {
    Pointer<Restraint> ret = IMP::internal::create_tuple_restraint
      (this, m, vt, get_name());
    ret->set_last_score(score);
    return Restraints(1, ret);
  }
}

Restraints ClassnameScore::create_current_decomposition(
    Model *m, PASSINDEXTYPE vt) const {
  return do_create_current_decomposition(m, vt);
}

IMPKERNEL_END_NAMESPACE
