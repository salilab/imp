/**
 *  \file NameDecorator.cpp   \brief Simple name decorator.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <sstream>

#include "IMP/decorators/NameDecorator.h"

namespace IMP
{

StringKey NameDecorator::name_key_;




void NameDecorator::show(std::ostream &out,std::string prefix) const
{
  out << prefix << get_name();

}




IMP_DECORATOR_INITIALIZE(NameDecorator, DecoratorBase,
                         {
                           name_key_= StringKey("name");
                         })

} // namespace IMP
