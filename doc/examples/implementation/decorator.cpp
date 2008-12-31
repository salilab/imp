/**
 *  \file MyDecorator.h     \brief Add a name to a particle.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPMODULENAME_MY_DECORATOR_H
#define IMPMODULENAME_MY_DECORATOR_H

#include "config.h"

#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/DecoratorBase.h>

IMPMODULENAME_BEGIN_NAMESPACE

//! A simple decorator which adds a name to a particle.
/** */
class IMPCOREEXPORT MyDecorator: public DecoratorBase
{
  static StringKey get_name_key();

public:
  //! Create a decorator from a named particle without error checking.
  MyDecorator(Particle *p);
  //! Create a decorator from a named particle
  static MyDecorator cast(Particle *p) {
    IMP_assert( p->has_attribute(get_name_key()), "Particle " << p->get_index()
               << " missing required attributes.",
               InvalidStateException);
    return MyDecorator(p);
  }

  //! Add a name to the particle
  static MyDecorator create(Particle *p, std::string name) {
    p->add_attribute(get_name_key(), name);
    MyDecorator ret(p);
    return ret;
  }

  //! Set the name of the particle
  void set_name(std::string name) {
    get_particle()->set_attribute(get_name_key(), name);
  }

  //! Get the name of the particle
  void get_name(std::string name) {
    get_particle()->get_attribute(get_name_key());
  }

  // Print out information about the particle
  void show(std::ostream &out) const {
    out << "Particle named " << get_name() << std::endl;
  }
};

// Make it so the C++ operator<< can be used.
IMP_OUTPUT_OPERATOR(MyDecorator);

IMPMODULENAME_END_NAMESPACE

#endif  /* IMPMODULENAME_MY_DECORATOR_H */


/**
 *  \file MyDecorator.cpp   \brief Add a name to a particle.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/modulename/MyDecorator.h>

IMPCORE_BEGIN_NAMESPACE

MyDecorator::MyDecorator(Particle *p): DecoratorBase(p) {
}

StringKey MyDecorator::get_name_key() {
  /* the compiler will make sure this is initialized the first time the
   method is called. */
  static StringKey mykey("my name");
  return mykey;
}

IMPCORE_END_NAMESPACE
