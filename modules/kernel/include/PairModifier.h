/**
 *  \file IMP/kernel/PairModifier.h
 *  \brief A Modifier on ParticlePairsTemp
 *
 *  WARNING This file was generated from NAMEModifier.hpp
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_PAIR_MODIFIER_H
#define IMPKERNEL_PAIR_MODIFIER_H

#include <IMP/kernel/kernel_config.h>
#include "DerivativeAccumulator.h"
#include "base_types.h"
#include "ParticleTuple.h"
#include "internal/container_helpers.h"
#include "input_output_macros.h"

IMPKERNEL_BEGIN_NAMESPACE

//! A base class for modifiers of ParticlePairsTemp
/** The primary function of such a class is to change
    the passed particles.

    \see IMP::PairFunctor

    Implementors should see IMP_PAIR_MODIFIER(). Also see
    PairDerivativeModifier.
 */
class IMPKERNELEXPORT PairModifier : public base::Object
{
public:
  typedef ParticlePair Argument;
  typedef ParticleIndexPair IndexArgument;
  PairModifier(std::string name="PairModifier %1%");

  /** Apply the function to a single value*/
  virtual void apply(const ParticlePair&) const =0;

 /** Apply the function to a single value*/
  virtual void apply_index(Model *m, const ParticleIndexPair& v) const {
    apply(internal::get_particle(m, v));
  }

  /** Apply the function to a collection of ParticlePairsTemp */
  /** If bounds are passed, only apply to ones between the upper and
      lower bounds.*/
  virtual void apply_indexes(Model *m, const ParticleIndexPairs &o,
                             unsigned int lower_bound,
                             unsigned int upper_bound) const {
    for (unsigned int i=lower_bound; i < upper_bound; ++i) {
      apply_index(m, o[i]);
    }
  }
  IMP_INPUTS_DECL(PairModifier);
  IMP_OUTPUTS_DECL(PairModifier);
};



IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_PAIR_MODIFIER_H */
