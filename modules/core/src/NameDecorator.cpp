/**
 *  \file NameDecorator.cpp   \brief Simple name decorator.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/NameDecorator.h>

#include <sstream>

#ifndef IMP_NO_DEPRECATED

IMPCORE_BEGIN_NAMESPACE

void NameDecorator::show(std::ostream &out,std::string prefix) const
{
  out << prefix << get_name();

}

StringKey NameDecorator::get_default_name_key() {
  static StringKey k("name");
  return k;
}

IMPCORE_END_NAMESPACE


#endif /* IMP_NO_DEPRECATED*/
