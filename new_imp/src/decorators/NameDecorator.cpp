/**
 *  \file NameDecorator.cpp   \brief Simple name decorator.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <IMP/decorators/NameDecorator.h>
#include <sstream>

namespace IMP
{

bool NameDecorator::keys_initialized_=false;
StringKey NameDecorator::name_key_;




void NameDecorator::show(std::ostream &out, int level) const
{
  out << get_name();

}





void NameDecorator::initialize_static_data()
{
  if (keys_initialized_) return;
  else {
    name_key_= StringKey("name");
    keys_initialized_=true;
  }
}

} // namespace IMP
