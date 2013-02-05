/**
 *  \file IMP/kernel/TripletModifier.h
 *  \brief A Modifier on ParticleTripletsTemp
 *
 *  WARNING This file was generated from NAMEModifier.hpp
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_TRIPLET_MODIFIER_H
#define IMPKERNEL_TRIPLET_MODIFIER_H

#include <IMP/kernel/kernel_config.h>
#include "DerivativeAccumulator.h"
#include "base_types.h"
#include "ParticleTuple.h"
#include "internal/container_helpers.h"
#include "input_output_macros.h"

IMPKERNEL_BEGIN_NAMESPACE

//! A base class for modifiers of ParticleTripletsTemp
/** The primary function of such a class is to change
    the passed particles.

    \see IMP::TripletFunctor

    Implementors should see IMP_TRIPLET_MODIFIER(). Also see
    TripletDerivativeModifier.
 */
class IMPKERNELEXPORT TripletModifier : public base::Object
{
public:
  typedef ParticleTriplet Argument;
  typedef ParticleIndexTriplet IndexArgument;
  TripletModifier(std::string name="TripletModifier %1%");

  /** Apply the function to a single value*/
  virtual void apply(const ParticleTriplet&) const =0;

 /** Apply the function to a single value*/
  virtual void apply_index(Model *m, const ParticleIndexTriplet& v) const {
    apply(internal::get_particle(m, v));
  }

  /** Apply the function to a collection of ParticleTripletsTemp */
  /** If bounds are passed, only apply to ones between the upper and
      lower bounds.*/
  virtual void apply_indexes(Model *m, const ParticleIndexTriplets &o,
                             unsigned int lower_bound,
                             unsigned int upper_bound) const {
    for (unsigned int i=lower_bound; i < upper_bound; ++i) {
      apply_index(m, o[i]);
    }
  }
  IMP_INPUTS_DECL(TripletModifier);
  IMP_OUTPUTS_DECL(TripletModifier);
};



IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_TRIPLET_MODIFIER_H */
