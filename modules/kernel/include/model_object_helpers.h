/**
 *  \file IMP/kernel/model_object_helpers.h    \brief Single variable function.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_MODEL_OBJECT_HELPERS_H
#define IMPKERNEL_MODEL_OBJECT_HELPERS_H

#include <IMP/kernel/kernel_config.h>
#include "ModelObject.h"
#include "particle_index.h"

IMPKERNEL_BEGIN_NAMESPACE

/** Inherit from this for objects that take particle arguments and read from
    them. */
class IMPKERNELEXPORT ParticleInputs {
public:
  /** Get the list of ModelObjects read when passed the given list of particles
      is used.*/
  ModelObjectsTemp get_inputs(Model *m,
                              const ParticleIndexes &pis) const;

  /** \deprecated Use get_inputs instead. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
      virtual ContainersTemp get_input_containers(Particle *p) const;
  /** \deprecated Use get_inputs instead. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
      virtual ParticlesTemp get_input_particles(Particle *p) const;
protected:
  /** Overload this method to specify the inputs.*/
  virtual ModelObjectsTemp do_get_inputs(Model *m,
                                 const ParticleIndexes &pis) const;
  virtual ~ParticleInputs(){}
};

/** Inherit from this for objects that take particle arguments and modify
    them. */
class IMPKERNELEXPORT ParticleOutputs {
public:
  /** Get the list of ModelObjects changed when passed the given list of
      particles is used.*/
  ModelObjectsTemp get_outputs(Model *m,
                              const ParticleIndexes &pis) const;

  /** \deprecated Use get_inputs instead. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
      virtual ContainersTemp get_output_containers(Particle *p) const;
  /** \deprecated Use get_inputs instead. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
      virtual ParticlesTemp get_output_particles(Particle *p) const;
protected:
  /** Overload this method to specify the outputs.*/
  virtual ModelObjectsTemp do_get_outputs(Model *m,
                                 const ParticleIndexes &pis) const;
  virtual ~ParticleOutputs(){}
};

/** Compute the set of score states required by the passed list of
    ModelObjects. This will compute dependencies if needed.*/
IMPKERNELEXPORT ScoreStatesTemp
get_required_score_states(const ModelObjectsTemp &mos,
                          ScoreStatesTemp exclude = ScoreStatesTemp());

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_MODEL_OBJECT_HELPERS_H */
