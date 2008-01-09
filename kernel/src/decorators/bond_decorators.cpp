/**
 *  \file bond_decorators.cpp     \brief Decorators for a bond
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
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

} // namespace internal


void BondDecorator::show(std::ostream &, std::string) const
{

}

void BondedDecorator::show(std::ostream &, std::string) const
{

}

static void bond_initialize_static_data()
{
  if (internal::bond_keys_initialized_) {
  } else {
    internal::bond_graph_data_= internal::GraphData("bond");
    internal::bond_length_key_=FloatKey("bond length");
    internal::bond_type_key_= IntKey("bond type");
    internal::bond_order_key_=IntKey("bond order");
    internal::bond_keys_initialized_=true;
  }
}

IMP_DECORATOR_INITIALIZE(BondDecorator, DecoratorBase,
                         bond_initialize_static_data());


IMP_DECORATOR_INITIALIZE(BondedDecorator, DecoratorBase,
                         bond_initialize_static_data());


BondDecorator bond(BondedDecorator a, BondedDecorator b, Int t)
{
  Particle *p= internal::graph_connect(a.get_particle(), b.get_particle(),
                    internal::bond_graph_data_);
  BondDecorator bd= BondDecorator::cast(p);
  bd.set_type(t);
  return bd;
}

Particles get_bonded(BondedDecorator a)
{
  Particles out;
  internal::graph_connected_component(a.get_particle(), 
                                      internal::bond_graph_data_, 
                                      std::back_inserter(out));
  return out;
}

} // namespace IMP
