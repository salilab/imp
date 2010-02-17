/**
 *  \file example/ExampleDecorator.cpp
 *  \brief Add a name to a particle.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/example/ExampleDecorator.h>

IMPEXAMPLE_BEGIN_NAMESPACE

StringKey ExampleDecorator::get_name_key() {
  /* the compiler will make sure this is initialized the first time the
   method is called. */
  static StringKey mykey("my name");
  return mykey;
}

void ExampleDecorator::show(std::ostream &out) const {
  out << get_name();
}

IMPEXAMPLE_END_NAMESPACE
