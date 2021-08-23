/**
 *  \file IMP/classname_macros.h
 *  \brief Macros for various classes.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_CLASSNAME_MACROS_H
#define IMPKERNEL_CLASSNAME_MACROS_H

#include "internal/TupleRestraint.h"
#include "internal/functors.h"
#include "container_macros.h"
#include <IMP/object_macros.h>
#include <algorithm>

/** Define
    - IMP::ClassnameScore::evaluate_indexes()
    - IMP::ClassnameScore::evaluate_if_good_indexes()
 */
#define IMP_CLASSNAME_SCORE_METHODS(Name)                                      \
  double evaluate_indexes(Model *m, const PLURALINDEXTYPE &p,                  \
                          DerivativeAccumulator *da, unsigned int lower_bound, \
                          unsigned int upper_bound)                            \
                          const IMP_OVERRIDE IMP_FINAL {                       \
    double ret = 0;                                                            \
    for (unsigned int i = lower_bound; i < upper_bound; ++i) {                 \
      ret += evaluate_index(m, p[i], da);                                      \
    }                                                                          \
    return ret;                                                                \
  }                                                                            \
  double evaluate_if_good_indexes(                                             \
      Model *m, const PLURALINDEXTYPE &p, DerivativeAccumulator *da,           \
      double max, unsigned int lower_bound,                                    \
      unsigned int upper_bound) const IMP_OVERRIDE {                           \
    double ret = 0;                                                            \
    for (unsigned int i = lower_bound; i < upper_bound; ++i) {                 \
      ret += evaluate_if_good_index(m, p[i], da, max - ret);                   \
      if (ret > max) return std::numeric_limits<double>::max();                \
    }                                                                          \
    return ret;                                                                \
  }

//! Define extra the functions needed for a ClassnamePredicate
#define IMP_CLASSNAME_PREDICATE_METHODS(Name)                                  \
  int get_value(ARGUMENTTYPE a) const {                                        \
    return get_value_index(IMP::internal::get_model(a),                        \
                           IMP::internal::get_index(a));                       \
  }                                                                            \
  Ints get_value(const PLURALVARIABLETYPE &o) const {                          \
    Ints ret(o.size());                                                        \
    for (unsigned int i = 0; i < o.size(); ++i) {                              \
      ret[i] += Name::get_value(o[i]);                                         \
    }                                                                          \
    return ret;                                                                \
  }                                                                            \
  Ints get_value_index(Model *m, const PLURALINDEXTYPE &o) const IMP_OVERRIDE {\
    Ints ret(o.size());                                                        \
    for (unsigned int i = 0; i < o.size(); ++i) {                              \
      ret[i] += Name::get_value_index(m, o[i]);                                \
    }                                                                          \
    return ret;                                                                \
  }                                                                            \
  IMP_IMPLEMENT_INLINE_NO_SWIG(                                                \
      void remove_if_equal(Model *m, PLURALINDEXTYPE &ps,                      \
                           int value) const,                                   \
  {                                                                            \
        ps.erase(                                                              \
            std::remove_if(ps.begin(), ps.end(),                               \
                           IMP::internal::PredicateEquals<Name, true>(         \
                               this, m, value)),                               \
            ps.end());                                                         \
      });                                                                      \
  IMP_IMPLEMENT_INLINE_NO_SWIG(void remove_if_not_equal(Model *m,              \
                                                        PLURALINDEXTYPE &ps,   \
                                                        int value) const,      \
  {                                                                            \
    ps.erase(                                                                  \
        std::remove_if(ps.begin(), ps.end(),                                   \
                       IMP::internal::PredicateEquals<Name, false>(            \
                           this, m, value)),                                   \
        ps.end());                                                             \
  });

//! Use IMP_CLASSNAME_MODIFIER() instead
#define IMP_CLASSNAME_DERIVATIVE_MODIFIER(Name) IMP_CLASSNAME_MODIFIER(Name)

/** Define
    - IMP::ClassnameModifier::apply_indexes()
*/
#define IMP_CLASSNAME_MODIFIER_METHODS(Name)                             \
  virtual void apply_indexes(Model *m, const PLURALINDEXTYPE &o,         \
                             unsigned int lower_bound,                   \
                             unsigned int upper_bound)                   \
                             const IMP_OVERRIDE IMP_FINAL {              \
    for (unsigned int i = lower_bound; i < upper_bound; ++i) {           \
      apply_index(m, o[i]);                                              \
    }                                                                    \
  }

//! Use IMP_INDEX_CLASSNAME_MODIFIER instead
#define IMP_INDEX_CLASSNAME_DERIVATIVE_MODIFIER(Name) \
  IMP_INDEX_CLASSNAME_MODIFIER(Name)

#ifndef IMP_DOXYGEN
#define IMP_IMPLEMENT_CLASSNAME_CONTAINER(Name)                          \
  IMP_IMPLEMENT_INLINE(void do_apply(const ClassnameModifier *sm) const, \
  { apply_generic(sm); });                                               \
  virtual ParticleIndexes get_all_possible_indexes() const IMP_OVERRIDE; \
  IMP_OBJECT_METHODS(Name)
#endif

/** Use this to fill in container methods
    IMP::ClassnameContainer::do_apply()
*/
#define IMP_CLASSNAME_CONTAINER_METHODS(Name) \
  void do_apply(const ClassnameModifier *sm) const IMP_OVERRIDE { \
    apply_generic(sm); }

#endif /* IMPKERNEL_CLASSNAME_MACROS_H */
