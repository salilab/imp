/**
 *  \file IMP/PairDerivativeModifier.h
 *  \brief A Modifier on ParticlePairsTemp
 *
 *  WARNING This file was generated from NAMEDerivativeModifier.hpp
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_PAIR_DERIVATIVE_MODIFIER_H
#define IMPKERNEL_PAIR_DERIVATIVE_MODIFIER_H

#include "kernel_config.h"
#include "DerivativeAccumulator.h"
#include "base_types.h"
#include "ParticleTuple.h"
#include "internal/container_helpers.h"
#include "input_output_macros.h"


IMP_BEGIN_NAMESPACE
// to keep swig happy
class Particle;

//! A base class for modifiers of ParticlePairsTemp
/** The primary function of such a class is to change
    the derivatives of the passed particles.

    Implementors should see and
    IMP_PAIR_DERIVATIVE_MODIFIER() and
    IMP::PairModifier.
 */
class IMPEXPORT PairDerivativeModifier : public base::Object
{
public:
  typedef ParticlePair Argument;
  typedef ParticleIndexPair IndexArgument;
  PairDerivativeModifier(std::string name="PairModifier %1%");

  /** Apply the function to a single value*/
  virtual void apply(const ParticlePair&,
                     DerivativeAccumulator &da) const=0;

 /** Apply the function to a single value*/
  virtual void apply_index(Model *m, const ParticleIndexPair& v,
                           DerivativeAccumulator &da) const {
    apply(internal::get_particle(m, v), da);
  }

  /** Apply the function to a collection of ParticlePairsTemp */
  virtual void apply_indexes(Model *m, const ParticleIndexPairs &o,
                             DerivativeAccumulator &da) const {
    for (unsigned int i=0; i < o.size(); ++i) {
      apply_index(m, o[i], da);
    }
  }

  IMP_INPUTS_DECL(PairModifier);
  IMP_OUTPUTS_DECL(PairModifier);
};


IMP_OBJECTS(PairDerivativeModifier,PairDerivativeModifiers);


IMP_END_NAMESPACE

#endif  /* IMPKERNEL_PAIR_DERIVATIVE_MODIFIER_H */
