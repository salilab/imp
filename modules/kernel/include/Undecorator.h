/**
 *  \file IMP/kernel/Undecorator.h    \brief The base class for decorators.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_UNDECORATOR_H
#define IMPKERNEL_UNDECORATOR_H

#include <IMP/kernel/kernel_config.h>
#include "base_types.h"
#include <IMP/base/Object.h>
#include "particle_index.h"

IMPKERNEL_BEGIN_NAMESPACE

class Model;

/** Undecorators can be registered with the model to tear down multi-particle
    invariants when a particle is removed from the model. Decorators that
    have such an invariant (eg an IMP::core::RigidBody) should register
    an Undecorator when setup_particle is called.
*/
class IMPKERNELEXPORT Undecorator: public base::Object {
  Model *m_;
 public:
  Undecorator(Model *m, std::string name);
  virtual void teardown(ParticleIndex pi) const = 0;
};

IMP_OBJECTS(Undecorator, Undecorators);


IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_UNDECORATOR_H */
