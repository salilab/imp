/**
 *  \file bond_decorators.cpp     \brief Decorators for a bond
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/atom/bond_decorators.h>

IMPATOM_BEGIN_NAMESPACE

namespace internal
{


BondData &get_bond_data() {
  static BondData d={IntKey("bond type"),
                     IntKey("bond order"),
                     FloatKey("bond length"),
                     FloatKey("bond stiffness"),
     IMP::core::internal::GraphData("bond")};
  return d;
}

} // namespace internal


void Bond::show(std::ostream &out) const
{
  if (*this == Bond()) {
    out << "Null Bond";
    return;
  }
  out << "Bond between "
      << get_bonded(0).get_particle()->get_name() << " and "
      << get_bonded(1).get_particle()->get_name();
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

void Bonded::show(std::ostream &out) const
{
  if (*this == Bonded()) {
    out << "Null Bonded";
    return;
  }
  out << "Particle " << get_particle()->get_name()
      << " is bonded to ";
  for (unsigned int i=0; i< get_number_of_bonds(); ++i){
    Bond b= get_bond(i);
    if (b.get_bonded(0) == *this) {
      out << b.get_bonded(1).get_particle()->get_name();
    } else  {
      out << b.get_bonded(0).get_particle()->get_name();
    }
    out << " ";
  }
}

Bond bond(Bonded a, Bonded b, Int t)
{
  IMP_USAGE_CHECK(a.get_particle() != b.get_particle(),
            "The endpoints of a bond must be disjoint",
            ValueException);

  Particle *p= IMP::core::internal::graph_connect(a.get_particle(),
                                                  b.get_particle(),
                                       internal::get_bond_data().graph_);
  Bond bd(p);
  bd.set_type(t);
  return bd;
}

void unbond(Bond b) {
  graph_disconnect(b.get_particle(), internal::get_bond_data().graph_);
}

Bond get_bond(Bonded a, Bonded b) {
  if (a==b) return Bond();
  for (unsigned int i=0; i < a.get_number_of_bonds(); ++i) {
    Bond bd= a.get_bond(i);
    if (bd.get_bonded(0) == b || bd.get_bonded(1) == b) {
      return bd;
    }
  }
  return Bond();
}

IMPATOM_END_NAMESPACE
