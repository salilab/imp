/**
 *  \file isd/NuisanceRangeModifier.cpp
 *  \brief A singleton modifier which wraps an attribute into a
 *  given range.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/isd/NuisanceRangeModifier.h"


IMPISD_BEGIN_NAMESPACE

void NuisanceRangeModifier::apply(Particle *p) const {
  Nuisance d(p);
  d.set_nuisance(d.get_nuisance());
}


/* Only the passed particle is used */
ParticlesTemp
NuisanceRangeModifier::get_input_particles(Particle *p) const {
  return ParticlesTemp(1, p);
}

ParticlesTemp
NuisanceRangeModifier::get_output_particles(Particle *p) const {
  return ParticlesTemp(1, p);
}

ContainersTemp
NuisanceRangeModifier::get_input_containers(Particle *) const {
  return ContainersTemp();
}

ContainersTemp
NuisanceRangeModifier::get_output_containers(Particle *) const {
  return ContainersTemp();
}

void NuisanceRangeModifier::do_show(std::ostream &out) const {
  out << "NuisanceRangeModifier " << std::endl;
}

IMPISD_END_NAMESPACE
