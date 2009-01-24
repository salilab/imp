/**
 *  \file ExampleDecorator.cpp   \brief Add a name to a particle.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
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

/* output the prefix first in order to get the spacing right when
   printing a hierarchy.
 */
void ExampleDecorator::show(std::ostream &out, std::string prefix) const {
  out << prefix << get_name();
}

IMPEXAMPLES_END_NAMESPACE
