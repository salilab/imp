/**
 *  \file IMP/example/ExampleDecorator.h     \brief Add a name to a particle.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEXAMPLE_EXAMPLE_DECORATOR_H
#define IMPEXAMPLE_EXAMPLE_DECORATOR_H

#include <IMP/example/example_config.h>

#include <IMP/kernel/Particle.h>
#include <IMP/Model.h>
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>
#include <IMP/base/exception.h>

IMPEXAMPLE_BEGIN_NAMESPACE

//! A simple decorator which adds a name to a particle.
/** A decorator adds functionality to a particle and ensures that invariants
    are preserved. In this case, the functionality is the setting and access
    of a name for the kernel::Particle and the invariant is that the name is always
    non-empty.

    The source code is as follows:
    \include ExampleDecorator.h
    \include ExampleDecorator.cpp
*/
class IMPEXAMPLEEXPORT ExampleDecorator : public Decorator {
  /* Use a static variable in a static method to create the key
     so that it is only done once and is only done when it is first
     needed. Lazy initialization of keys makes \imp more efficient as
     kernel::Particles do not have to allocate memory for ununsed keys.
  */
  static StringKey get_name_key();
  //! Add a name to the particle
  /** The create function should take arguments which allow
      the initial state of the Decorator to be reasonable (i.e.
      make sure there is a non-empty name).
   */
  static void do_setup_particle(Model *m, kernel::ParticleIndex pi, std::string name) {
    // use the usage check macro to make sure that arguments are correct
    IMP_USAGE_CHECK(!name.empty(), "The name cannot be empty.");
    m->add_attribute(get_name_key(), pi, name);
  }

 public:
  //! return true if the particle has a name
  static bool get_is_setup(Model *m, kernel::ParticleIndex pi) {
    return m->get_has_attribute(get_name_key(), pi);
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
  IMP_DECORATOR_METHODS(ExampleDecorator, Decorator);
  IMP_DECORATOR_SETUP_1(ExampleDecorator, std::string, name);
};

/** Define a collection of them. Also look at example.i*/
IMP_DECORATORS(ExampleDecorator, ExampleDecorators, kernel::Particles);

IMPEXAMPLE_END_NAMESPACE

#endif /* IMPEXAMPLE_EXAMPLE_DECORATOR_H */
