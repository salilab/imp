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
#include "IMP/kernel/internal/InternalDynamicListClassnameContainer.h"
#include "IMP/kernel/internal/InternalListClassnameContainer.h"
#include "IMP/kernel/internal/utility.h"
#include "IMP/kernel/ModelObject.h"
#include <algorithm>
#include <IMP/base/check_macros.h>
#include <IMP/base/set.h>
#include <IMP/kernel/classname_macros.h>
#include <IMP/kernel/ClassnameModifier.h>
#include <IMP/kernel/ClassnamePredicate.h>
#include <IMP/kernel/ClassnameScore.h>
#include <IMP/kernel/functor.h>
#include <IMP/kernel/internal/ListLikeClassnameContainer.h>
#include <IMP/kernel/internal/TupleRestraint.h>
#include <IMP/kernel/internal/utility.h>
#include <IMP/kernel/Restraint.h>




IMPKERNEL_BEGIN_NAMESPACE


ClassnameContainer::ClassnameContainer(Model *m, std::string name):
  Container(m,name){
}

// here for gcc
ClassnameContainer::~ClassnameContainer(){
}

PLURALVARIABLETYPE ClassnameContainer
::get_FUNCTIONNAMEs() const {
  return IMP::kernel::internal::get_particle(get_model(),
                                     get_indexes());
}

#if IMP_HAS_DEPRECATED
bool ClassnameContainer
::get_contains_FUNCTIONNAME(VARIABLETYPE v) const {
  IMP_DEPRECATED_FUNCTION(something else);
  INDEXTYPE iv= IMP::kernel::internal::get_index(v);
  IMP_FOREACH_CLASSNAME_INDEX(this, {
      if (_1 == iv) return true;
    });
  return false;
}

unsigned int ClassnameContainer
::get_number_of_FUNCTIONNAMEs() const {
  IMP_DEPRECATED_FUNCTION(IMP_CONTAINER_FOREACH());
  return get_number();
}

VARIABLETYPE ClassnameContainer
::get_FUNCTIONNAME(unsigned int i) const {
  IMP_DEPRECATED_FUNCTION(IMP_CONTAINER_FOREACH());
  return get(i);
}
#endif

bool ClassnameContainer
::get_provides_access() const {
  validate_readable();
  return do_get_provides_access();
}

void ClassnameContainer
::apply_generic(const ClassnameModifier *m) const {
  apply(m);
}

void ClassnameContainer
::apply(const ClassnameModifier *sm) const {
  validate_readable();
  do_apply(sm);
}


ClassnameContainerAdaptor
::ClassnameContainerAdaptor(ClassnameContainer *c): P(c){}
ClassnameContainerAdaptor
::ClassnameContainerAdaptor(const PLURALVARIABLETYPE &t,
                                                 std::string name) {
  Model *m=internal::get_model(t);
  IMP_NEW(internal::InternalListClassnameContainer, c,
          (m, name));
  c->set(IMP::kernel::internal::get_index(t));
  P::operator=(c);
}



ClassnameModifier::ClassnameModifier(std::string name):
  Object(name){
}

IMP_INPUTS_DEF(ClassnameModifier);
IMP_OUTPUTS_DEF(ClassnameModifier);

ClassnamePredicate::ClassnamePredicate(std::string name):
  Object(name)
{
  /* Implemented here rather than in the header so that PairPredicate
     symbols are present in the kernel DSO */
}

void ClassnamePredicate::remove_if_equal(Model *m,
                                         PLURALINDEXTYPE& ps,
                                         int value) const {
  ps.erase(std::remove_if(ps.begin(), ps.end(),
                          make_predicate_equal(this, m, value)),
           ps.end());

}

void ClassnamePredicate::remove_if_not_equal(Model *m,
                                             PLURALINDEXTYPE& ps,
                                             int value) const {
  ps.erase(std::remove_if(ps.begin(), ps.end(),
                          make_predicate_not_equal(this, m, value)),
           ps.end());
}

IMP_INPUTS_DEF(ClassnamePredicate);

ClassnameScore::ClassnameScore(std::string name):
  Object(name)
{
  /* Implemented here rather than in the header so that PairScore
     symbols are present in the kernel DSO */
}

double ClassnameScore::evaluate(ARGUMENTTYPE vt,
                                DerivativeAccumulator *da) const {
  return evaluate_index(internal::get_model(vt),
                        internal::get_index(vt),
                        da);
}

// old versions of gcc don't like having the pragma inside the function
IMP_DEPRECATED_IGNORE(
double ClassnameScore::evaluate_index(Model *m, PASSINDEXTYPE vt,
                                      DerivativeAccumulator *da) const {
  // see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53469
   return evaluate(internal::get_particle(m, vt), da);
})

double ClassnameScore::evaluate_indexes(Model *m,
                                        const PLURALINDEXTYPE &o,
                                        DerivativeAccumulator *da,
                                        unsigned int lower_bound,
                                        unsigned int upper_bound) const {
  double ret=0;
  for (unsigned int i=lower_bound; i< upper_bound; ++i) {
    ret+= evaluate_index(m, o[i], da);
  }
  return ret;
}


double ClassnameScore::evaluate_if_good_index(Model *m,
                                              PASSINDEXTYPE vt,
                                              DerivativeAccumulator *da,
                                              double max)
  const {
  IMP_UNUSED(max);
  return evaluate_index(m, vt, da);
}

double ClassnameScore::evaluate_if_good_indexes(Model *m,
                                                const PLURALINDEXTYPE &o,
                                                DerivativeAccumulator *da,
                                                double max,
                                                unsigned int lower_bound,
                                                unsigned int upper_bound)
  const {
  double ret=0;
  for (unsigned int i=lower_bound; i< upper_bound; ++i) {
    double cur= evaluate_if_good_index(m, o[i], da, max-ret);
    max-=cur;
    ret+=cur;
    if (max<0) break;
  }
  return ret;
}


Restraints
ClassnameScore
::do_create_current_decomposition(Model *m,
                                  PASSINDEXTYPE vt) const {
  double score=evaluate_index(m, vt, nullptr);
  if (score==0) {
    return Restraints();
  } else {
    return Restraints(1, IMP::kernel::internal::create_tuple_restraint(this,
                                                               m,
                                                               vt,
                                                               get_name()));
  }
}

Restraints
ClassnameScore
::create_current_decomposition(Model *m,
                               PASSINDEXTYPE vt) const {
  return do_create_current_decomposition(m, vt);
}

IMP_INPUTS_DEF(ClassnameScore);

IMPKERNEL_END_NAMESPACE
IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

InternalDynamicListClassnameContainer
::InternalDynamicListClassnameContainer(Container *m,
                                        std::string name):
    P(m->get_model(), name), scope_(m) {
}


InternalDynamicListClassnameContainer
::InternalDynamicListClassnameContainer(Container *m,
                                        const char *name):
    P(m->get_model(), name), scope_(m) {
}


void InternalDynamicListClassnameContainer::do_show(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  out << get_access()
      << " Classnames." << std::endl;
}
void InternalDynamicListClassnameContainer::add(PASSINDEXTYPE vt) {
  PLURALINDEXTYPE cur;
  swap(cur);
  cur.push_back(vt);
  swap(cur);
}
void InternalDynamicListClassnameContainer
::add(const PLURALINDEXTYPE &c) {
  if (c.empty()) return;
  PLURALINDEXTYPE cur;
  swap(cur);
  cur+=c;
  swap(cur);
}

void InternalDynamicListClassnameContainer::set(PLURALINDEXTYPE cp) {
  swap(cp);
}
void InternalDynamicListClassnameContainer::clear() {
  PLURALINDEXTYPE t;
  swap(t);
}
bool InternalDynamicListClassnameContainer::
check_list(const ParticleIndexes& cp) const {
  ParticleIndexes app
    = scope_->get_all_possible_indexes();

  base::set<ParticleIndex> all(app.begin(),
                                    app.end());
  for (unsigned int i=0; i< cp.size(); ++i) {
    IMP_USAGE_CHECK(all.find(cp[i]) != all.end(),
                    "Particle " << cp[i]
                    << " is not in the list of all possible particles");
  }
  return true;
}

ParticleIndexes
InternalDynamicListClassnameContainer::get_all_possible_indexes() const {
  return scope_->get_all_possible_indexes();
}

void InternalDynamicListClassnameContainer::do_before_evaluate() {
}


ParticlesTemp
InternalDynamicListClassnameContainer::get_input_particles() const {
  return ParticlesTemp();
}

ContainersTemp
InternalDynamicListClassnameContainer::get_input_containers() const {
  return ContainersTemp();
}


PLURALINDEXTYPE
InternalDynamicListClassnameContainer::get_range_indexes() const {
  return get_indexes();
}


void ListLikeClassnameContainer
::do_show(std::ostream &out) const {
  out << "contains " << data_.size() << std::endl;
}



InternalListClassnameContainer
::InternalListClassnameContainer(Model *m, std::string name):
  P(m, name){
}


InternalListClassnameContainer
::InternalListClassnameContainer(Model *m, const char *name):
  P(m, name){
}
void InternalListClassnameContainer::add(PASSINDEXTYPE vt) {
  get_model()->clear_caches();
  PLURALINDEXTYPE cur;
  swap(cur);
  cur.push_back(vt);
  swap(cur);
}
void InternalListClassnameContainer
::add(const PLURALINDEXTYPE &c) {
  if (c.empty()) return;
  get_model()->clear_caches();
  PLURALINDEXTYPE cur;
  swap(cur);
  cur+=c;
  swap(cur);
}
void InternalListClassnameContainer::set(PLURALINDEXTYPE cp) {
  get_model()->clear_caches();
  swap(cp);
}
void InternalListClassnameContainer::clear() {
  get_model()->clear_caches();
  PLURALINDEXTYPE t;
  swap(t);
}
void InternalListClassnameContainer::remove(PASSINDEXTYPE vt) {
  get_model()->clear_caches();
  PLURALINDEXTYPE t;
  swap(t);
  t.erase(std::remove(t.begin(), t.end(), vt), t.end());
  swap(t);
}
void InternalListClassnameContainer::do_show(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  out << get_access()
      << " Classnames." << std::endl;
}


ParticleIndexes
InternalListClassnameContainer::get_all_possible_indexes() const {
  return IMP::kernel::internal::flatten(get_indexes());
}

PLURALINDEXTYPE
InternalListClassnameContainer::get_range_indexes() const {
  return get_indexes();
}

void InternalListClassnameContainer::do_before_evaluate() {
}


ParticlesTemp
InternalListClassnameContainer::get_input_particles() const {
  return ParticlesTemp();
}

ContainersTemp
InternalListClassnameContainer::get_input_containers() const {
  return ContainersTemp();
}

IMPKERNEL_END_INTERNAL_NAMESPACE
