/**
 *  \file Particle.cpp   \brief Classes to handle individual model particles.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Model.h"
#include "IMP/internal/utility.h"

IMP_BEGIN_NAMESPACE

namespace {
  unsigned int next_index=0;
}

Particle::Particle(Model *m, std::string name)
{
  m->add_particle_internal(this);
  set_name(internal::make_object_name(name, next_index++));
}


void Particle::zero_derivatives()
{
  derivatives_.set_values(0);
}

void Particle::show(std::ostream& out) const
{
  const std::string inset("  ");
  out << std::endl;
  out << "Particle: " << get_name() << std::endl;
  if (get_is_active()) {
    out << inset << inset << "active";
  } else {
    out << inset << inset << "dead";
  }
  out << std::endl;

  if (model_) {
    out << inset << "float attributes:" << std::endl;
    floats_.show(out, inset+inset);

    out << inset << "float derivatives:" << std::endl;
    derivatives_.show(out, inset+inset);

    out << inset << "optimizeds:" << std::endl;
    optimizeds_.show(out, inset+inset);

    out << inset << "int attributes:" << std::endl;
    ints_.show(out, inset+inset);

    out << inset << "string attributes:" << std::endl;
    strings_.show(out, inset+inset);

    out << inset << "particle attributes:" << std::endl;
    particles_.show(out, inset+inset);

  }
}

IMP_END_NAMESPACE
