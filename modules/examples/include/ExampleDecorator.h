/**
 *  \file ExampleDecorator.h     \brief Add a name to a particle.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPEXAMPLES_EXAMPLE_DECORATOR_H
#define IMPEXAMPLES_EXAMPLE_DECORATOR_H

#include "config.h"

#include <IMP/core/macros.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/Decorator.h>
#include <IMP/exception.h>

IMPEXAMPLES_BEGIN_NAMESPACE

//! A simple decorator which adds a name to a particle.
/** A decorator adds functionality to a particle and ensures that invariants
    are preserved. In this case, the functionality is the setting and access
    of a name for the Particle and the invariant is that the name is always
    non-empty.

    The source code is as follows:
    \include ExampleDecorator.h
    \include ExampleDecorator.cpp
*/
class IMPEXAMPLESEXPORT ExampleDecorator: public Decorator
{
  /* Use a static variable in a static method to create the key
     so that it is only done once and is only done when it is first
     needed. Lazy initialization of keys makes IMP more efficient as
     Particles do not have to allocate memory for ununsed keys.
  */
  static StringKey get_name_key();

public:

  //! Add a name to the particle
  /** The create function should take arguments which allow
      the initial state of the Decorator to be reasonable (i.e.
      make sure there is a non-empty name).
   */
  static ExampleDecorator create(Particle *p, std::string name) {
    IMP_check(!name.empty(), "The name cannot be empty.",
              ValueException);
    p->add_attribute(get_name_key(), name);
    ExampleDecorator ret(p);
    return ret;
  }

  //! return true if the particle has a name
  static bool is_instance_of(Particle *p) {
    return p->has_attribute(get_name_key());
  }
  //! Set the name of the particle
  void set_name(std::string name) {
    IMP_check(!name.empty(), "The name cannot be empty.",
              ValueException);
    get_particle()->set_value(get_name_key(), name);
  }

  //! Get the name of the particle
  std::string get_name() const {
    return get_particle()->get_value(get_name_key());
  }
  /* Declare the basic constructors and the cast function.*/
  IMP_DECORATOR(ExampleDecorator, Decorator)
};

// Make it so the C++ operator<< can be used.
IMP_OUTPUT_OPERATOR(ExampleDecorator);

IMPEXAMPLES_END_NAMESPACE

#endif  /* IMPEXAMPLES_EXAMPLE_DECORATOR_H */
