/**
 *  \file MolecularHierarchyDecorator.cpp   \brief Decorator for helping deal
 *                                                 with a hierarchy.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <IMP/decorators/MolecularHierarchyDecorator.h>
#include <sstream>

namespace IMP
{

bool MolecularHierarchyDecorator::keys_initialized_=false;
IntKey MolecularHierarchyDecorator::type_key_;

void MolecularHierarchyDecorator::show(std::ostream &out) const
{
  P::show(out);
  out << "Type is " << get_type_string() <<std::endl;
}




void MolecularHierarchyDecorator::initialize_static_data()
{
  if (keys_initialized_) {
    return;
  } else {
    P::initialize_static_data();
    type_key_=IntKey("molecular hierarchy type");
    keys_initialized_=true;
  }
}

} // namespace IMP
