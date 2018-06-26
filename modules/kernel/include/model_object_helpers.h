/**
 *  \file IMP/model_object_helpers.h
 *  \brief Classes used in the construction of ModelObjects
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_MODEL_OBJECT_HELPERS_H
#define IMPKERNEL_MODEL_OBJECT_HELPERS_H

#include <IMP/kernel_config.h>
#include "ModelObject.h"
#include "particle_index.h"
#include "ScoreState.h"
IMPKERNEL_BEGIN_NAMESPACE

//! Base class for objects that take particle arguments and read from them
class IMPKERNELEXPORT ParticleInputs {
 public:
  //! Get the ModelObjects read when the given list of particles is used.
  ModelObjectsTemp get_inputs(Model *m,
                              const ParticleIndexes &pis) const;

 protected:
  //! Overload this method to specify the inputs.
  virtual ModelObjectsTemp do_get_inputs(Model *m,
                                         const ParticleIndexes &pis) const = 0;
  virtual ~ParticleInputs() {}
};

//! Base class for objects that take particle arguments and modify them
class IMPKERNELEXPORT ParticleOutputs {
 public:
  //! Get the ModelObjects changed when the given list of particles is used.
  ModelObjectsTemp get_outputs(Model *m,
                               const ParticleIndexes &pis) const;

 protected:
  //! Overload this method to specify the outputs.
  virtual ModelObjectsTemp do_get_outputs(Model *m,
                                          const ParticleIndexes &pis) const = 0;
  virtual ~ParticleOutputs() {}
};

//! Compute the set of score states required by the passed ModelObjects.
/** This will compute dependencies if needed.
 */
IMPKERNELEXPORT ScoreStatesTemp
    get_required_score_states(const ModelObjectsTemp &mos,
                              ScoreStatesTemp exclude = ScoreStatesTemp());

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_MODEL_OBJECT_HELPERS_H */
