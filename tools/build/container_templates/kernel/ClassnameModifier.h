/**
 *  \file IMP/ClassnameModifier.h
 *  \brief A Modifier on PLURALVARIABLETYPE
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_CLASSNAME_MODIFIER_H
#define IMPKERNEL_CLASSNAME_MODIFIER_H

#include <IMP/kernel_config.h>
#include "DerivativeAccumulator.h"
#include "base_types.h"
#include "internal/container_helpers.h"
#include "model_object_helpers.h"

IMPKERNEL_BEGIN_NAMESPACE

//! A base class for modifiers of PLURALVARIABLETYPE
/** The primary function of such a class is to change
    the passed particles.

    \see IMP::ClassnamePredicate

    Implementers should see IMP_CLASSNAME_MODIFIER(). Also see
    ClassnameDerivativeModifier.
 */
class IMPKERNELEXPORT ClassnameModifier : public ParticleInputs,
                                          public ParticleOutputs,
                                          public Object {
 public:
  typedef VARIABLETYPE Argument;
  typedef INDEXTYPE IndexArgument;
  ClassnameModifier(std::string name = "ClassnameModifier %1%");

  /** Apply the function to a single value*/
  virtual void apply_index(Model *m, PASSINDEXTYPE v) const = 0;

  /** Apply the function to a collection of PLURALVARIABLETYPE */
  /** If bounds are passed, only apply to ones between the upper and
      lower bounds.*/
  virtual void apply_indexes(Model *m, const PLURALINDEXTYPE &o,
                             unsigned int lower_bound,
                             unsigned int upper_bound) const {
    for (unsigned int i = lower_bound; i < upper_bound; ++i) {
      apply_index(m, o[i]);
    }
  }

  /** Apply the function to a collection of PLURALVARIABLETYPE */
  /** Information on particles that moved since the last scoring function
      evaluation is also passed, which may be used to accelerate the
      computation. */
  virtual void apply_indexes_moved(
              Model *m, const PLURALINDEXTYPE &o,
              unsigned int lower_bound, unsigned int upper_bound,
              const ParticleIndexes &moved_pis,
              const ParticleIndexes &reset_pis) const {
    IMP_UNUSED(moved_pis);
    IMP_UNUSED(reset_pis);
    apply_indexes(m, o, lower_bound, upper_bound);
  }
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_CLASSNAME_MODIFIER_H */
