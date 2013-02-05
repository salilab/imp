/**
 *  \file IMP/kernel/CLASSNAMEModifier.h
 *  \brief A Modifier on PLURALVARIABLETYPE
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_HEADERNAME_MODIFIER_H
#define IMPKERNEL_HEADERNAME_MODIFIER_H

#include <IMP/kernel/kernel_config.h>
#include "DerivativeAccumulator.h"
#include "base_types.h"
#include "ParticleTuple.h"
#include "internal/container_helpers.h"
#include "input_output_macros.h"

IMPKERNEL_BEGIN_NAMESPACE

//! A base class for modifiers of PLURALVARIABLETYPE
/** The primary function of such a class is to change
    the passed particles.

    \see IMP::CLASSNAMEFunctor

    Implementors should see IMP_HEADERNAME_MODIFIER(). Also see
    CLASSNAMEDerivativeModifier.
 */
class IMPKERNELEXPORT CLASSNAMEModifier : public base::Object
{
public:
  typedef VARIABLETYPE Argument;
  typedef INDEXTYPE IndexArgument;
  CLASSNAMEModifier(std::string name="CLASSNAMEModifier %1%");

  /** Apply the function to a single value*/
  virtual void apply(ARGUMENTTYPE) const =0;

 /** Apply the function to a single value*/
  virtual void apply_index(Model *m, PASSINDEXTYPE v) const {
    apply(internal::get_particle(m, v));
  }

  /** Apply the function to a collection of PLURALVARIABLETYPE */
  /** If bounds are passed, only apply to ones between the upper and
      lower bounds.*/
  virtual void apply_indexes(Model *m, const PLURALINDEXTYPE &o,
                             unsigned int lower_bound,
                             unsigned int upper_bound) const {
    for (unsigned int i=lower_bound; i < upper_bound; ++i) {
      apply_index(m, o[i]);
    }
  }
  IMP_INPUTS_DECL(CLASSNAMEModifier);
  IMP_OUTPUTS_DECL(CLASSNAMEModifier);
};



IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_HEADERNAME_MODIFIER_H */
