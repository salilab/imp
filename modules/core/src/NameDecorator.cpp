/**
 *  \file NameDecorator.cpp   \brief Simple name decorator.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/NameDecorator.h>

#include <sstream>

IMPCORE_BEGIN_NAMESPACE

StringKey NameDecorator::name_key_;




void NameDecorator::show(std::ostream &out,std::string prefix) const
{
  out << prefix << get_name();

}




IMP_DECORATOR_INITIALIZE(NameDecorator, DecoratorBase,
                         {
                           name_key_= StringKey("name");
                         })

IMPCORE_END_NAMESPACE
