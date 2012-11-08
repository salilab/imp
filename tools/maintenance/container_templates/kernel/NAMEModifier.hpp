/**
 *  \file IMP/CLASSNAMEModifier.h
 *  \brief A Modifier on PLURALVARIABLETYPE
 *
 *  BLURB
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_HEADERNAME_MODIFIER_H
#define IMPKERNEL_HEADERNAME_MODIFIER_H

#include "kernel_config.h"
#include "DerivativeAccumulator.h"
#include "base_types.h"
#include "ParticleTuple.h"
#include "internal/container_helpers.h"
#include "input_output_macros.h"

IMP_BEGIN_NAMESPACE

//! A base class for modifiers of PLURALVARIABLETYPE
/** The primary function of such a class is to change
    the passed particles.

    \see IMP::CLASSNAMEFunctor

    Implementors should see IMP_HEADERNAME_MODIFIER(). Also see
    CLASSNAMEDerivativeModifier.
 */
class IMPEXPORT CLASSNAMEModifier : public base::Object
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

  //! Enable them to be use as functors
  /** But beware of slicing.
   */
  int operator()(Model *m, PASSINDEXTYPE vt) const {
    return apply_index(m, vt);
  }

  //! Enable them to be use as functors
  /** But beware of slicing.
   */
  int operator()(Model *m, const PLURALINDEXTYPE &o) const {
    return apply_indexes(m, o);
  }

  /** Apply the function to a collection of PLURALVARIABLETYPE */
  virtual void apply_indexes(Model *m, const PLURALINDEXTYPE &o) const {
    for (unsigned int i=0; i < o.size(); ++i) {
      apply_index(m, o[i]);
    }
  }
  IMP_INPUTS_DECL(CLASSNAMEModifier);
  IMP_OUTPUTS_DECL(CLASSNAMEModifier);
};



IMP_END_NAMESPACE

#endif  /* IMPKERNEL_HEADERNAME_MODIFIER_H */
