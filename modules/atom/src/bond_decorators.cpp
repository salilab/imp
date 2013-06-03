/**
 *  \file bond_decorators.cpp     \brief Decorators for a bond
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/bond_decorators.h>

IMPATOM_BEGIN_NAMESPACE

namespace internal {

BondData &get_bond_data() {
  static BondData d = {IntKey("bond type"), IntKey("bond order"),
                       FloatKey("bond length"), FloatKey("bond stiffness"),
                       IMP::core::internal::GraphData("bond")};
  return d;
}

}  // namespace internal

void Bond::show(std::ostream &out) const {
  if (*this == Bond()) {
    out << "Null Bond";
    return;
  }
  out << "Bond between " << get_bonded(0).get_particle()->get_name() << " and "
      << get_bonded(1).get_particle()->get_name();
  if (get_type() != NONBIOLOGICAL) {
    out << " of type " << get_type();
  }
  if (get_order() != 1) out << " and order " << get_order();
  if (get_particle()->has_attribute(internal::get_bond_data().length_)) {
    out << " and length "
        << get_particle()->get_value(internal::get_bond_data().length_);
  }
  out << std::endl;
}

void Bonded::show(std::ostream &out) const {
  if (*this == Bonded()) {
    out << "Null Bonded";
    return;
  }
  out << "Particle " << get_particle()->get_name() << " is bonded to ";
  for (unsigned int i = 0; i < get_number_of_bonds(); ++i) {
    Bond b = get_bond(i);
    if (b.get_bonded(0) == *this) {
      out << b.get_bonded(1).get_particle()->get_name();
    } else {
      out << b.get_bonded(0).get_particle()->get_name();
    }
    out << " ";
  }
}

Bond create_bond(Bonded a, Bonded b, Int t) {
  IMP_USAGE_CHECK(a.get_particle() != b.get_particle(),
                  "The endpoints of a bond must be disjoint");

  Particle *p = IMP::core::internal::graph_connect(
      a.get_particle(), b.get_particle(), internal::get_bond_data().graph_);
  Bond bd(p);
  bd.set_type(t);
  return bd;
}

void destroy_bond(Bond b) {
  graph_disconnect(b.get_particle(), internal::get_bond_data().graph_);
}

/* This is implemented like this so that it doesn't read any particles other
   than a and b. To do otherwise would make it rather annoying to use in
   evaluate.
*/
Bond get_bond(Bonded a, Bonded b) {
  if (a == b) return Bond();
  ParticleIndexes ba = a.get_bonds();
  ParticleIndexes bb = b.get_bonds();
  std::sort(bb.begin(), bb.end());
  for (unsigned int i = 0; i < ba.size(); ++i) {
    if (std::binary_search(bb.begin(), bb.end(), ba[i])) {
      return Bond(a.get_model(), ba[i]);
    }
  }
  return Bond();
}

namespace {
bool check_bond(Particle *p) {
  if (p->get_value(internal::get_bond_data().length_) < 0) {
    IMP_THROW("Invalid bond length: "
                  << p->get_value(internal::get_bond_data().length_),
              ValueException);
  }
  if (p->get_value(internal::get_bond_data().stiffness_) < 0) {
    IMP_THROW("Invalid bond stiffness: "
                  << p->get_value(internal::get_bond_data().stiffness_),
              ValueException);
  }
  Bond bd(p);
  for (unsigned int j = 0; j < 2; ++j) {
    Bonded bdd = bd.get_bonded(j);
    bool found = false;
    for (unsigned int i = 0; i < bdd.get_number_of_bonds(); ++i) {
      if (bdd.get_bond(i) == bd) {
        found = true;
        break;
      }
    }
    if (!found) {
      IMP_THROW("Inconsistent bond: " << bd, ValueException);
    }
  }
  return true;
}
}

IMP_CHECK_DECORATOR(Bond, check_bond);

namespace {
bool check_bonded(Particle *p) {
  Bonded bdd(p);
  for (unsigned int i = 0; i < bdd.get_number_of_bonds(); ++i) {
    if (bdd.get_bond(i).get_bonded(0) != bdd &&
        bdd.get_bond(i).get_bonded(1) != bdd) {
      IMP_THROW("Invalid bond at " << bdd << ", " << i, ValueException);
    }
  }
  return true;
}
}

IMP_CHECK_DECORATOR(Bonded, check_bonded);

IMPATOM_END_NAMESPACE
