/**
 *  \file Nuisance.cpp
 *  \brief defines a one-dimensional nuisance parameter (sigma, gamma, ...)
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/isd/Nuisance.h"

IMPISD_BEGIN_NAMESPACE

void Nuisance::show(std::ostream &out) const {
  out << " < Nuisance " << get_nuisance() <<  std::endl;
}



IMPISD_END_NAMESPACE
