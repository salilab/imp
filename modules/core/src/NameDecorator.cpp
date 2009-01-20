/**
 *  \file NameDecorator.cpp   \brief Simple name decorator.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/NameDecorator.h>

#include <sstream>

IMPCORE_BEGIN_NAMESPACE

StringKey NameDecorator::get_name_key() {
  static StringKey k("name");
  return k;
}




void NameDecorator::show(std::ostream &out,std::string prefix) const
{
  out << prefix << get_name();

}






IMPCORE_END_NAMESPACE
