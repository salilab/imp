/**
 *  \file Decorator.h    \brief The base class for decorators.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_DECORATOR_H
#define IMP_DECORATOR_H

#include "Object.h"
#include "Pointer.h"
#include "Particle.h"
#include "Model.h"

IMP_BEGIN_NAMESPACE

/** A base class for decorators. To read more about decorators go to the
    \ref decorators "Decorator introduction".

    \note Decorator objects are ordered based on the address of the wrapped
    particle.

    \cpp Implementers of decorators should
    just inherit from this and then use the IMP_DECORATOR macro to
    provide the key implementation pieces.\n\n
    Remember that attribute keys should always be created lazily
    (at the time of the first use), and not be created as static variables.\n\n
    Implementors should consult IMP::examples::ExampleDecorator, IMP_DECORATOR()
    IMP_DECORATOR_TRAITS(), IMP_DECORATOR_GET(), IMP_DECORATOR_ARRAY_DECL()
*/
class Decorator
{
private:
  Pointer<Particle> particle_;
protected:
  bool is_default() const {
    return !particle_;
  }
  Decorator(Particle *p): particle_(p) {}
  Decorator() {}
public:
  IMP_NO_DOXYGEN(typedef Decorator This;)
 /** \name Methods provided by the Decorator class
      The following methods are provided by the Decorator class.
      @{
  */
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
  // here just to make the docs my symmetric
private:
  IMP_ONLY_DOXYGEN(int blah_;)
  //! @}
public:
#ifdef IMP_DOXYGEN

  /** \name Methods that all decorators must have
      All decorators must have the following methods. Decorators
      which are parameterized (for example IMP::core::XYZRDecorator)
      take an (optional) extra parameter after the Particle in
      create(), cast() and is_instance_of().
      Note that these are
      not actually methods of the Decorator class itself.
      @{
  */
  /** Add the needed attributes to the particle and initialize them
      with values taken from initial_values.

      \throw InvalidStateException if the Particle has already been set up
  */
  static Decorator create(Particle *p, extra_arguments);

  /** Create a decorator from a particle which has already been set up.
      That is, the Decorator::create() function was previously called with
      the Particle and so it has all the needed attributes.
      \throw InvalidStateException if p cannot be cast successfully.
  */
  static Decorator cast(Particle *p);

  /** Return true if the particle can be cast to the decorator. */
  static bool is_instance_of(Particle *p);

  /** Write a description of the wrapped Particle as seen by
      the decorator to a stream. Each line should be prefixed by
      prefix.
  */
  void show(std::ostream &out, std::string prefix) const;

  /** Create an instance of the Decorator from the particle which already
      has the needed attributes. The key difference between this constructor
      and cast() is that there is not necessarily any error checking performed.
  */
  Decorator(Particle *p);
  /** The default constructor must be defined and create a NULL decorator,
      analogous to a \c NULL pointer in C++ or a \c None object in Python.
  */
  Decorator();
  //! @}
#endif
};

IMP_END_NAMESPACE

#endif  /* IMP_DECORATOR_H */
