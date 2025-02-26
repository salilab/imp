/**
 *  \file internal/LennardJonesParameters.cpp
 *  \brief Precomputed Lennard-Jones terms between particle pairs.
 *
 *  Copyright 2007-2025 IMP Inventors. All rights reserved.
 */

#include <IMP/atom/atom_config.h>
#include <IMP/atom/LennardJonesTypedPairScore.h>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

void LennardJonesParameters::precalculate(int i) {
  LennardJonesType *ti = get(i);
  for (int j = 0; j < types_.size(); ++j) {
    LennardJonesType *tj = get(j);
    int ind = get_parameter_index(i, j);

    double well_depth = std::sqrt(ti->get_well_depth() * tj->get_well_depth());
    double rmin = ti->get_radius() + tj->get_radius();
    // probably faster than pow(rmin, 6) on systems that don't
    // have pow(double, int)
    double rmin6 = rmin * rmin * rmin * rmin * rmin * rmin;
    double rmin12 = rmin6 * rmin6;

    aij_[ind] = well_depth * rmin12;
    bij_[ind] = 2.0 * well_depth * rmin6;
  }
}

int LennardJonesParameters::add(LennardJonesType *typ) {
  types_.push_back(typ);
  int index = types_.size() - 1;
  // fill in aij, bij for new type interacting with all previous types
  size_t newsz = get_parameter_index(index, index) + 1;
  aij_.resize(newsz);
  bij_.resize(newsz);
  precalculate(index);
  return index;
}

LennardJonesType* LennardJonesParameters::get(int index) {
  return types_[index];
}

LennardJonesParameters* get_lj_params() {
  static PointerMember<LennardJonesParameters>
              lj = new LennardJonesParameters();
  return lj;
}

IMPATOM_END_INTERNAL_NAMESPACE
