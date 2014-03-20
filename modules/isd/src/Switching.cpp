/**
 *  \file Switching.h
 *  \brief A decorator for switching parameters particles
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#include <IMP/isd/Switching.h>

IMPISD_BEGIN_NAMESPACE

void Switching::show(std::ostream &out) const {
  out << get_lower() << " < ";
  out << " Switching = " << get_nuisance();
  out << " < " << get_upper();
}

IMPISD_END_NAMESPACE
