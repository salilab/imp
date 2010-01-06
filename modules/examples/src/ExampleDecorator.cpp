/**
 *  \file ExampleDecorator.cpp   \brief Add a name to a particle.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/examples/ExampleDecorator.h>

IMPEXAMPLES_BEGIN_NAMESPACE

StringKey ExampleDecorator::get_name_key() {
  /* the compiler will make sure this is initialized the first time the
   method is called. */
  static StringKey mykey("my name");
  return mykey;
}

void ExampleDecorator::show(std::ostream &out) const {
  out << get_name();
}

IMPEXAMPLES_END_NAMESPACE
