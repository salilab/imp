/**
 *  \file IMP/kernel/ClassnamePredicate.h
 *  \brief Define ClassnamePredicate.
 *
 *  BLURB
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_CLASSNAME_PREDICATE_H
#define IMPKERNEL_CLASSNAME_PREDICATE_H

#include <IMP/kernel/kernel_config.h>
#include "base_types.h"
#include "ParticleTuple.h"
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
                                           public base::Object {
 public:
  typedef VARIABLETYPE Argument;
  typedef INDEXTYPE IndexArgument;
  ClassnamePredicate(std::string name = "ClassnamePredicate %1%");
  /** \deprecated_at{2.1} Use the index based version.*/
  IMPKERNEL_DEPRECATED_METHOD_DECL(2.1)
  virtual int get_value(ARGUMENTTYPE vt) const;

  /** \deprecated_at{2.1} Use the index based version.*/
  IMPKERNEL_DEPRECATED_METHOD_DECL(2.1)
  virtual Ints get_value(const PLURALVARIABLETYPE &o) const;

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  virtual void remove_if_equal(kernel::Model *m, PLURALINDEXTYPE &ps,
                               int v) const;
  virtual void remove_if_not_equal(kernel::Model *m, PLURALINDEXTYPE &ps,
                                   int v) const;
#endif

  //! Compute the predicate and the derivative if needed.
  virtual int get_value_index(kernel::Model *m, PASSINDEXTYPE vt) const;

  //! Enable them to be use as functors
  /** But beware of slicing.
   */
  int operator()(kernel::Model *m, PASSINDEXTYPE vt) const {
    return get_value_index(m, vt);
  }

  virtual Ints get_value_index(kernel::Model *m,
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
