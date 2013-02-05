/**
 *  \file IMP/kernel/QuadModifier.h
 *  \brief A Modifier on ParticleQuadsTemp
 *
 *  WARNING This file was generated from NAMEModifier.hpp
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_QUAD_MODIFIER_H
#define IMPKERNEL_QUAD_MODIFIER_H

#include <IMP/kernel/kernel_config.h>
#include "DerivativeAccumulator.h"
#include "base_types.h"
#include "ParticleTuple.h"
#include "internal/container_helpers.h"
#include "input_output_macros.h"

IMPKERNEL_BEGIN_NAMESPACE

//! A base class for modifiers of ParticleQuadsTemp
/** The primary function of such a class is to change
    the passed particles.

    \see IMP::QuadFunctor

    Implementors should see IMP_QUAD_MODIFIER(). Also see
    QuadDerivativeModifier.
 */
class IMPKERNELEXPORT QuadModifier : public base::Object
{
public:
  typedef ParticleQuad Argument;
  typedef ParticleIndexQuad IndexArgument;
  QuadModifier(std::string name="QuadModifier %1%");

  /** Apply the function to a single value*/
  virtual void apply(const ParticleQuad&) const =0;

 /** Apply the function to a single value*/
  virtual void apply_index(Model *m, const ParticleIndexQuad& v) const {
    apply(internal::get_particle(m, v));
  }

  /** Apply the function to a collection of ParticleQuadsTemp */
  /** If bounds are passed, only apply to ones between the upper and
      lower bounds.*/
  virtual void apply_indexes(Model *m, const ParticleIndexQuads &o,
                             unsigned int lower_bound,
                             unsigned int upper_bound) const {
    for (unsigned int i=lower_bound; i < upper_bound; ++i) {
      apply_index(m, o[i]);
    }
  }
  IMP_INPUTS_DECL(QuadModifier);
  IMP_OUTPUTS_DECL(QuadModifier);
};



IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_QUAD_MODIFIER_H */
