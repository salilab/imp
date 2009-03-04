/**
 *  \file estimates.cpp
 *  \brief Estimates of various physical quantities.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
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

IMPATOM_END_NAMESPACE
