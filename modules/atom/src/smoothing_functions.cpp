/**
 *  \file smoothing_functions.cpp    Classes to smooth nonbonded interactions
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/atom/smoothing_functions.h>

IMPATOM_BEGIN_NAMESPACE

SmoothingFunction::SmoothingFunction() : Object("SmoothingFunction%1%") {}

void ForceSwitch::do_show(std::ostream &out) const {
  out << "switching between " << min_distance_ << " and " << max_distance_;
}

IMPATOM_END_NAMESPACE
