/**
 *  \file bond_decorators.cpp     \brief Decorators for a bond
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <IMP/decorators/bond_decorators.h>

namespace IMP
{


namespace internal {
GraphData bond_graph_data_;
bool bond_keys_initialized_=false;
FloatKey bond_length_key_;
IntKey bond_type_key_;
IntKey bond_order_key_;
}


void BondDecorator::show(std::ostream &) const
{

}

void BondedDecorator::show(std::ostream &) const
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

void BondDecorator::initialize_static_data()
{
  bond_initialize_static_data();
}


void BondedDecorator::initialize_static_data()
{
  bond_initialize_static_data();
}


BondDecorator bond(BondedDecorator a, BondedDecorator b)
{
  ParticleIndex pi= internal::graph_connect(a.get_particle(), b.get_particle(),
                    internal::bond_graph_data_);
  Particle *p= a.get_model()->get_particle(pi);
  return BondDecorator::cast(p);
}
}
