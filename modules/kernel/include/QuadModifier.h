/**
 *  \file IMP/QuadModifier.h
 *  \brief A Modifier on ParticleQuadsTemp
 *
 *  WARNING This file was generated from NAMEModifier.hpp
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_QUAD_MODIFIER_H
#define IMPKERNEL_QUAD_MODIFIER_H

#include "kernel_config.h"
#include "DerivativeAccumulator.h"
#include "base_types.h"
#include "ParticleTuple.h"
#include "internal/container_helpers.h"
#include "input_output_macros.h"

IMP_BEGIN_NAMESPACE

//! A base class for modifiers of ParticleQuadsTemp
/** The primary function of such a class is to change
    the passed particles.

    \see IMP::QuadFunctor

    Implementors should see IMP_QUAD_MODIFIER(). Also see
    QuadDerivativeModifier.
 */
class IMPEXPORT QuadModifier : public base::Object
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

  //! Enable them to be use as functors
  /** But beware of slicing.
   */
  void operator()(Model *m, const ParticleIndexQuad& vt) const {
    return apply_index(m, vt);
  }

 //! Enable them to be use as functors
  /** But beware of slicing.
   */
  void operator()(Model *m, const ParticleIndexQuads &o) const {
    return apply_indexes(m, o);
  }

  /** Apply the function to a collection of ParticleQuadsTemp */
  virtual void apply_indexes(Model *m, const ParticleIndexQuads &o) const {
    for (unsigned int i=0; i < o.size(); ++i) {
      apply_index(m, o[i]);
    }
  }
  IMP_INPUTS_DECL(QuadModifier);
  IMP_OUTPUTS_DECL(QuadModifier);
};



IMP_END_NAMESPACE

#endif  /* IMPKERNEL_QUAD_MODIFIER_H */
