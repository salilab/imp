/**
 *  \file isd/ScaleRangeModifier.cpp
 *  \brief A singleton modifier which wraps an attribute into a
 *  given range.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/isd/ScaleRangeModifier.h"
#include "IMP/isd/Scale.h"


IMPISD_BEGIN_NAMESPACE

void ScaleRangeModifier::apply(Particle *p) const {
  Scale d(p);
  d.set_scale(d.get_scale());
}


/* Only the passed particle is used */
ParticlesTemp
ScaleRangeModifier::get_input_particles(Particle *p) const {
  return ParticlesTemp(1, p);
}

ParticlesTemp
ScaleRangeModifier::get_output_particles(Particle *p) const {
  return ParticlesTemp(1, p);
}

ContainersTemp
ScaleRangeModifier::get_input_containers(Particle *) const {
  return ContainersTemp();
}

ContainersTemp
ScaleRangeModifier::get_output_containers(Particle *) const {
  return ContainersTemp();
}

void ScaleRangeModifier::do_show(std::ostream &out) const {
  out << "ScaleRangeModifier " << std::endl;
}

IMPISD_END_NAMESPACE
