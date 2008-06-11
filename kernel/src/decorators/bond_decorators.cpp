/**
 *  \file bond_decorators.cpp     \brief Decorators for a bond
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/decorators/bond_decorators.h"

namespace IMP
{


namespace internal
{

GraphData bond_graph_data_;
bool bond_keys_initialized_=false;
FloatKey bond_length_key_;
IntKey bond_type_key_;
IntKey bond_order_key_;
FloatKey bond_stiffness_key_;

} // namespace internal


void BondDecorator::show(std::ostream &out, std::string) const
{
  out << "Bond between " 
      << get_bonded(0).get_particle()->get_index() << " and "
      << get_bonded(1).get_particle()->get_index();
  if (get_type() != CUSTOM) {
    out << " of type " << get_type();
  }
  if (get_order() != 1) out << " and order " << get_order();
  if (get_particle()->has_attribute(internal::bond_length_key_)) {
    out << " and length "
        << get_particle()->get_value(internal::bond_length_key_);
  }
  out << std::endl;
}

void BondedDecorator::show(std::ostream &out, std::string) const
{
  out << "Particle " << get_particle()->get_index() 
      << " is bonded to ";
  for (unsigned int i=0; i< get_number_of_bonds(); ++i){
    BondDecorator b= get_bond(i);
    if (b.get_bonded(0) == *this) {
      out << b.get_bonded(1).get_particle()->get_index();
    } else  {
      out << b.get_bonded(0).get_particle()->get_index();
    }
    out << " ";                              
  }
}

static void bond_initialize_static_data()
{
  if (internal::bond_keys_initialized_) {
  } else {
    internal::bond_graph_data_= internal::GraphData("bond");
    internal::bond_type_key_= IntKey("bond type");
    internal::bond_order_key_=IntKey("bond order");
    internal::bond_length_key_=FloatKey("bond length");
    internal::bond_stiffness_key_=FloatKey("bond stiffness");
    internal::bond_keys_initialized_=true;
  }
}

IMP_DECORATOR_INITIALIZE(BondDecorator, DecoratorBase,
                         bond_initialize_static_data());


IMP_DECORATOR_INITIALIZE(BondedDecorator, DecoratorBase,
                         bond_initialize_static_data());


BondDecorator bond(BondedDecorator a, BondedDecorator b, Int t)
{
  IMP_check(a.get_particle() != b.get_particle(),
            "The endpoints of a bond must be disjoint",
            ValueException);

  Particle *p= internal::graph_connect(a.get_particle(), b.get_particle(),
                    internal::bond_graph_data_);
  BondDecorator bd(p);
  bd.set_type(t);
  return bd;
}

void unbond(BondDecorator b) {
  graph_disconnect(b.get_particle(), internal::bond_graph_data_);
}

BondDecorator get_bond(BondedDecorator a, BondedDecorator b) {
  for (unsigned int i=0; i < a.get_number_of_bonds(); ++i) {
    BondDecorator bd= a.get_bond(i);
    if (bd.get_bonded(0) == b || bd.get_bonded(1) == b) {
      return bd;
    }
  }
  return BondDecorator();
}

} // namespace IMP
