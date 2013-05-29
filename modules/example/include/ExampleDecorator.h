/**
 *  \file IMP/example/ExampleDecorator.h     \brief Add a name to a particle.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEXAMPLE_EXAMPLE_DECORATOR_H
#define IMPEXAMPLE_EXAMPLE_DECORATOR_H

#include <IMP/example/example_config.h>

#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>
#include <IMP/exception.h>

IMPEXAMPLE_BEGIN_NAMESPACE

//! A simple decorator which adds a name to a particle.
/** A decorator adds functionality to a particle and ensures that invariants
    are preserved. In this case, the functionality is the setting and access
    of a name for the Particle and the invariant is that the name is always
    non-empty.

    The source code is as follows:
    \include ExampleDecorator.h
    \include ExampleDecorator.cpp
*/
class IMPEXAMPLEEXPORT ExampleDecorator : public Decorator {
  /* Use a static variable in a static method to create the key
     so that it is only done once and is only done when it is first
     needed. Lazy initialization of keys makes \imp more efficient as
     Particles do not have to allocate memory for ununsed keys.
  */
  static StringKey get_name_key();

 public:

  //! Add a name to the particle
  /** The create function should take arguments which allow
      the initial state of the Decorator to be reasonable (i.e.
      make sure there is a non-empty name).
   */
  static ExampleDecorator setup_particle(Particle *p, std::string name) {
    // use the object check macro to check that the particle is valid
    IMP_CHECK_OBJECT(p);
    // use the usage check macro to make sure that arguments are correct
    IMP_USAGE_CHECK(!name.empty(), "The name cannot be empty.");
    p->add_attribute(get_name_key(), name);
    ExampleDecorator ret(p);
    return ret;
  }

  //! return true if the particle has a name
  static bool particle_is_instance(Particle *p) {
    return p->has_attribute(get_name_key());
  }

  //! Get the name added to the particle
  std::string get_decorator_name() const {
    return get_particle()->get_value(get_name_key());
  }

  //! Set the name added to the particle
  void set_decorator_name(std::string nm) {
    // use the usage check macro to check that functions are called properly
    IMP_USAGE_CHECK(!nm.empty(), "The name cannot be empty");
    get_particle()->set_value(get_name_key(), nm);
  }

  /* Declare the basic constructors and the cast function.*/
  IMP_DECORATOR(ExampleDecorator, Decorator);
};

/** Define a collection of them. Also look at example.i*/
IMP_DECORATORS(ExampleDecorator, ExampleDecorators, Particles);

IMPEXAMPLE_END_NAMESPACE

#endif /* IMPEXAMPLE_EXAMPLE_DECORATOR_H */
