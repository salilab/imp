/**
 *  \file IMP/CLASSNAMEDerivativeModifier.h
 *  \brief A Modifier on PLURALVARIABLETYPE
 *
 *  BLURB
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_HEADERNAME_DERIVATIVE_MODIFIER_H
#define IMPKERNEL_HEADERNAME_DERIVATIVE_MODIFIER_H

#include "kernel_config.h"
#include "DerivativeAccumulator.h"
#include "base_types.h"
#include "ParticleTuple.h"
#include "internal/container_helpers.h"
#include "input_output_macros.h"


IMP_BEGIN_NAMESPACE
<<<<<<< HEAD
// to keep swig happy
class Particle;

//! A base class for modifiers of PLURALVARIABLETYPE
/** The primary function of such a class is to change
    the derivatives of the passed particles.

    Implementors should see and
    IMP_HEADERNAME_DERIVATIVE_MODIFIER() and
    IMP::CLASSNAMEModifier.
 */
class IMPEXPORT CLASSNAMEDerivativeModifier : public base::Object
{
public:
  typedef VARIABLETYPE Argument;
  typedef INDEXTYPE IndexArgument;
  CLASSNAMEDerivativeModifier(std::string name="CLASSNAMEModifier %1%");

  /** Apply the function to a single value*/
  virtual void apply(ARGUMENTTYPE,
                     DerivativeAccumulator &da) const=0;

 /** Apply the function to a single value*/
  virtual void apply_index(Model *m, PASSINDEXTYPE v,
                           DerivativeAccumulator &da) const {
    apply(internal::get_particle(m, v), da);
  }

  //! Enable them to be use as functors
  /** But beware of slicing.

      The derivative weight is assumed to be 1.
   */
  void operator()(Model *m, PASSINDEXTYPE vt) const {
    DerivativeAccumulator da;
    return apply_index(m, vt, da);
  }

  //! Enable them to be use as functors
  /** But beware of slicing.
   */
  void operator()(Model *m, const PLURALINDEXTYPE &o) const {
    DerivativeAccumulator da;
    return apply_indexes(m, o, da);
  }

  /** Apply the function to a collection of PLURALVARIABLETYPE */
  virtual void apply_indexes(Model *m, const PLURALINDEXTYPE &o,
                             DerivativeAccumulator &da) const {
    for (unsigned int i=0; i < o.size(); ++i) {
      apply_index(m, o[i], da);
    }
  }

  IMP_INPUTS_DECL(CLASSNAMEModifier);
  IMP_OUTPUTS_DECL(CLASSNAMEModifier);
};


IMP_OBJECTS(CLASSNAMEDerivativeModifier,CLASSNAMEDerivativeModifiers);

=======
typedef CLASSNAMEModifier CLASSNAMEDerivativeModifier;
>>>>>>> merge Derivative modifiers with regular ones since the derivagive weight always has to be 0 and this simplifies a lot of things

IMP_END_NAMESPACE

#endif  /* IMPKERNEL_HEADERNAME_DERIVATIVE_MODIFIER_H */
