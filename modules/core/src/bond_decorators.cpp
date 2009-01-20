/**
 *  \file bond_decorators.cpp     \brief Decorators for a bond
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/bond_decorators.h>

IMPCORE_BEGIN_NAMESPACE

namespace internal
{


BondData &get_bond_data() {
  static BondData d={IntKey("bond type"),
      IntKey("bond order"),
    FloatKey("bond length"),
    FloatKey("bond stiffness"),
        internal::GraphData("bond")};
  return d;
}

} // namespace internal


void BondDecorator::show(std::ostream &out, std::string) const
{
  if (is_default()) {
    out << "Null BondDecorator";
    return;
  }
  out << "Bond between "
      << get_bonded(0).get_particle()->get_index() << " and "
      << get_bonded(1).get_particle()->get_index();
  if (get_type() != CUSTOM) {
    out << " of type " << get_type();
  }
  if (get_order() != 1) out << " and order " << get_order();
  if (get_particle()->has_attribute(internal::get_bond_data().length_)) {
    out << " and length "
        << get_particle()->get_value(internal::get_bond_data().length_);
  }
  out << std::endl;
}

void BondedDecorator::show(std::ostream &out, std::string) const
{
  if (!is_default()) {
    out << "Null BondedDecorator";
    return;
  }
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

BondDecorator bond(BondedDecorator a, BondedDecorator b, Int t)
{
  IMP_check(a.get_particle() != b.get_particle(),
            "The endpoints of a bond must be disjoint",
            ValueException);

  Particle *p= internal::graph_connect(a.get_particle(), b.get_particle(),
                                       internal::get_bond_data().graph_);
  BondDecorator bd(p);
  bd.set_type(t);
  return bd;
}

void unbond(BondDecorator b) {
  graph_disconnect(b.get_particle(), internal::get_bond_data().graph_);
}

BondDecorator get_bond(BondedDecorator a, BondedDecorator b) {
  if (a==b) return BondDecorator();
  for (unsigned int i=0; i < a.get_number_of_bonds(); ++i) {
    BondDecorator bd= a.get_bond(i);
    if (bd.get_bonded(0) == b || bd.get_bonded(1) == b) {
      return bd;
    }
  }
  return BondDecorator();
}

IMPCORE_END_NAMESPACE
