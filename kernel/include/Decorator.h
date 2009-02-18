/**
 *  \file Decorator.h    \brief The base class for decorators.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_DECORATOR_H
#define IMP_DECORATOR_H

#include "Object.h"
#include "Pointer.h"
#include "Particle.h"
#include "Model.h"

IMP_BEGIN_NAMESPACE

/** A base class for decorators. Implementers of decorators should
    just inherit from this and then use the IMP_DECORATOR macro to
    provide the key implementation. If you do not use the IMP_DECORATOR
    macro, you should instead implement
    - MyDecorator(Particle *p) which creates a decorator from a particle
      which is assumed to have the needed attributes.
    - MyDecorator::cast(Particle*p) which creates a decorator from a particle
      after checking that it has the needed attributes. An InvalidStateException
      should be thrown if it does not.
    - MyDecroator::create(Particle *p) which adds the required attributes to
      particle which is assumed not to have them and throws an
      InvalidStateException if there is a problem.

    \note Decorator objects are ordered based on the get_particle()

    \note Remember that attribute keys should always be created lazily
    (at the time of the first use), and not be created as static variables.
*/
class Decorator
{
protected:
  Pointer<Particle> particle_;
  Decorator(Particle *p): particle_(p) {}
  bool is_default() const {
    return !particle_;
  }
public:
  typedef Decorator This;

  Decorator() {}
  IMP_COMPARISONS_1(particle_);

  /** \return the particle wrapped by this decorator*/
  Particle *get_particle() const {
    return particle_.get();
  }
  /** \return the Model containing the particle */
  Model *get_model() const {
    IMP_CHECK_OBJECT(particle_->get_model());
    return particle_->get_model();
  }

};

IMP_END_NAMESPACE

#endif  /* IMP_DECORATOR_H */
