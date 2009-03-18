/**
 *  \file estimates.cpp
 *  \brief Estimates of various physical quantities.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include "IMP/atom/estimates.h"
#include <IMP/internal/units.h>

IMPATOM_BEGIN_NAMESPACE

double volume_in_A3_from_mass_in_kDa(double m) {
  using namespace IMP::unit;
  GramPerCubicCentimeter density(1.410+.145 *std::exp(-m/13.0));
  Kilodalton mkd(m);
  CubicAngstrom ca(convert_to_mks(mkd)/density);
  return ca.get_value();
}

double mass_in_kDa_from_number_of_residues(unsigned int num_aa) {
  static const double factor= 53.0/466.0;
  return factor*num_aa;
}

IMPATOM_END_NAMESPACE
