/**
 *  \file IMP/kernel/SingletonModifier.h
 *  \brief A Modifier on ParticlesTemp
 *
 *  WARNING This file was generated from NAMEModifier.hpp
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_SINGLETON_MODIFIER_H
#define IMPKERNEL_SINGLETON_MODIFIER_H

#include <IMP/kernel/kernel_config.h>
#include "DerivativeAccumulator.h"
#include "base_types.h"
#include "ParticleTuple.h"
#include "internal/container_helpers.h"
#include "input_output_macros.h"

IMPKERNEL_BEGIN_NAMESPACE

//! A base class for modifiers of ParticlesTemp
/** The primary function of such a class is to change
    the passed particles.

    \see IMP::SingletonFunctor

    Implementors should see IMP_SINGLETON_MODIFIER(). Also see
    SingletonDerivativeModifier.
 */
class IMPKERNELEXPORT SingletonModifier : public base::Object
{
public:
  typedef Particle* Argument;
  typedef ParticleIndex IndexArgument;
  SingletonModifier(std::string name="SingletonModifier %1%");

  /** Apply the function to a single value*/
  virtual void apply(Particle*) const =0;

 /** Apply the function to a single value*/
  virtual void apply_index(Model *m, ParticleIndex v) const {
    apply(internal::get_particle(m, v));
  }

  /** Apply the function to a collection of ParticlesTemp */
  /** If bounds are passed, only apply to ones between the upper and
      lower bounds.*/
  virtual void apply_indexes(Model *m, const ParticleIndexes &o,
                             unsigned int lower_bound,
                             unsigned int upper_bound) const {
    for (unsigned int i=lower_bound; i < upper_bound; ++i) {
      apply_index(m, o[i]);
    }
  }
  IMP_INPUTS_DECL(SingletonModifier);
  IMP_OUTPUTS_DECL(SingletonModifier);
};



IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_SINGLETON_MODIFIER_H */
