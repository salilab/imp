/**
 *  \file IMP/QuadDerivativeModifier.h
 *  \brief A Modifier on ParticleQuadsTemp
 *
 *  WARNING This file was generated from NAMEDerivativeModifier.hpp
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_QUAD_DERIVATIVE_MODIFIER_H
#define IMPKERNEL_QUAD_DERIVATIVE_MODIFIER_H

#include "kernel_config.h"
#include "DerivativeAccumulator.h"
#include "base_types.h"
#include "ParticleTuple.h"
#include "internal/container_helpers.h"
#include "input_output_macros.h"


IMP_BEGIN_NAMESPACE
// to keep swig happy
class Particle;

//! A base class for modifiers of ParticleQuadsTemp
/** The primary function of such a class is to change
    the derivatives of the passed particles.

    Implementors should see and
    IMP_QUAD_DERIVATIVE_MODIFIER() and
    IMP::QuadModifier.
 */
class IMPEXPORT QuadDerivativeModifier : public base::Object
{
public:
  typedef ParticleQuad Argument;
  typedef ParticleIndexQuad IndexArgument;
  QuadDerivativeModifier(std::string name="QuadModifier %1%");

  /** Apply the function to a single value*/
  virtual void apply(const ParticleQuad&,
                     DerivativeAccumulator &da) const=0;

 /** Apply the function to a single value*/
  virtual void apply_index(Model *m, const ParticleIndexQuad& v,
                           DerivativeAccumulator &da) const {
    apply(internal::get_particle(m, v), da);
  }

  //! Enable them to be use as functors
  /** But beware of slicing.

      The derivative weight is assumed to be 1.
   */
  void operator()(Model *m, const ParticleIndexQuad& vt) const {
    DerivativeAccumulator da;
    return apply_index(m, vt, da);
  }

  //! Enable them to be use as functors
  /** But beware of slicing.
   */
  void operator()(Model *m, const ParticleIndexQuads &o) const {
    DerivativeAccumulator da;
    return apply_indexes(m, o, da);
  }

  /** Apply the function to a collection of ParticleQuadsTemp */
  virtual void apply_indexes(Model *m, const ParticleIndexQuads &o,
                             DerivativeAccumulator &da) const {
    for (unsigned int i=0; i < o.size(); ++i) {
      apply_index(m, o[i], da);
    }
  }

  IMP_INPUTS_DECL(QuadModifier);
  IMP_OUTPUTS_DECL(QuadModifier);
};


IMP_OBJECTS(QuadDerivativeModifier,QuadDerivativeModifiers);


IMP_END_NAMESPACE

#endif  /* IMPKERNEL_QUAD_DERIVATIVE_MODIFIER_H */
