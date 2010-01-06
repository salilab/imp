/**
 *  \file estimates.cpp
 *  \brief Estimates of various physical quantities.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#include "IMP/atom/estimates.h"
#include <IMP/internal/units.h>

IMPATOM_BEGIN_NAMESPACE

double volume_from_mass(double m) {
  // Alber et al r = 0.726*std::pow(m, .3333);
  return 1.60*m;
}

double mass_from_number_of_residues(unsigned int num_aa) {
  return 110.0*num_aa;
}

IMPATOM_END_NAMESPACE
