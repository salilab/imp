/**
 *  \file IMP/ClassnamePredicate.h
 *  \brief Define ClassnamePredicate.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_CLASSNAME_PREDICATE_H
#define IMPKERNEL_CLASSNAME_PREDICATE_H

#include <IMP/kernel_config.h>
#include "base_types.h"
#include "DerivativeAccumulator.h"
#include "internal/container_helpers.h"
#include "model_object_helpers.h"

IMPKERNEL_BEGIN_NAMESPACE

//! Abstract predicate function
/** A predicate is a function which returns one of a discrete set of
    values (eg -1, 0, 1 depending on whether a value is negative, zero
    or positive). ClassnamePredicates will evaluate the predicate for the passed
    particles.

    Implementers should check out IMP_CLASSNAME_PREDICATE().
*/
class IMPKERNELEXPORT ClassnamePredicate : public ParticleInputs,
                                           public Object {
 public:
  typedef VARIABLETYPE Argument;
  typedef INDEXTYPE IndexArgument;
  ClassnamePredicate(std::string name = "ClassnamePredicate %1%");

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  //! remove any particles in ps for which this predicate evaluates to v
  virtual void remove_if_equal(Model *m, PLURALINDEXTYPE &ps,
                               int v) const;
  //! remove any particles in ps for which this predicate does not evaluate to v
  virtual void remove_if_not_equal(Model *m, PLURALINDEXTYPE &ps,
                                   int v) const;
#endif

  //! Compute the predicate and the derivative if needed.
  virtual int get_value_index(Model *m, PASSINDEXTYPE vt) const = 0;

  //! Setup for a batch of calls to get_value_index_in_batch()
  //! (could be used for improving performance - e.g. preload various
  //!  tables from model)
  virtual void setup_for_get_value_index_in_batch(Model*) const {};

  //! Same as get_value_index, but possibly with optimizations
  //! for a batch of calls. Call setup_for_get_value_index_in_batch()
  //! right before calling a batch of those.
  virtual int get_value_index_in_batch(Model* m, PASSINDEXTYPE vt) const{
    return get_value_index(m, vt);
  }

  //! Enable them to be use as functors
  /** But beware of slicing.
   */
  int operator()(Model *m, PASSINDEXTYPE vt) const {
    return get_value_index(m, vt);
  }

  virtual Ints get_value_index(Model *m,
                               const PLURALINDEXTYPE &o) const {
    Ints ret(o.size());
    for (unsigned int i = 0; i < o.size(); ++i) {
      ret[i] += get_value_index(m, o[i]);
    }
    return ret;
  }

  IMP_REF_COUNTED_DESTRUCTOR(ClassnamePredicate);
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_CLASSNAME_PREDICATE_H */
