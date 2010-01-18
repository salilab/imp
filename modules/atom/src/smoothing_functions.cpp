/**
 *  \file smoothing_functions.cpp    Classes to smooth nonbonded interactions
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#include <IMP/atom/smoothing_functions.h>

IMPATOM_BEGIN_NAMESPACE

SmoothingFunction::SmoothingFunction() {}

void ForceSwitch::show(std::ostream &out) const {
  out << "ForceSwitch switching between " << min_distance_
      << " and " << max_distance_;
}

IMPATOM_END_NAMESPACE
