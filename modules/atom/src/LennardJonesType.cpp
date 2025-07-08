/**
 *  \file LennardJonesType.cpp
 *  \brief Parameters for a Lennard-Jones interaction.
 *
 *  Copyright 2007-2025 IMP Inventors. All rights reserved.
 */

#include <IMP/atom/LennardJonesType.h>
#include <IMP/atom/smoothing_functions.h>

IMPATOM_BEGIN_NAMESPACE

LennardJonesType::LennardJonesType(double well_depth, double radius,
                                   std::string name)
                : Object(name), well_depth_(well_depth), radius_(radius) {
  index_ = internal::get_lj_params()->add(this);
}

void LennardJonesType::set_well_depth(double d) {
  well_depth_ = d;
  internal::get_lj_params()->precalculate(index_);
}

void LennardJonesType::set_radius(double r) {
  radius_ = r;
  internal::get_lj_params()->precalculate(index_);
}

IntKey LennardJonesTyped::get_type_key() {
  static IntKey k("lennard_jones_type");
  return k;
}

void LennardJonesTyped::show(std::ostream &out) const {
  XYZ::show(out);
  out << " Lennard-Jones type= " << get_index();
}

IMPATOM_END_NAMESPACE
