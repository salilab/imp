/**
 *  \file Particle.cpp   \brief Classes to handle individual model particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Model.h"

IMP_BEGIN_NAMESPACE

Particle::Particle(Model *m)
{
  is_active_ = true;
  m->add_particle(this);
}


Particle::Particle()
{
  is_active_ = true;
}

Particle::~Particle()
{
}


void Particle::set_model(Model *md, ParticleIndex pi)
{
  IMP_check(model_==NULL || md==NULL,
            "Set_model called for particle already in model",
            ValueException);
  model_ = md;
  pi_ = pi;
  IMP_check(model_==NULL || model_->get_particle(pi_)== this,
            "Set_model called with inconsistent data",
            ValueException);
}

void Particle::set_is_active(const bool is_active)
{
  is_active_ = is_active;
}


void Particle::zero_derivatives()
{
  derivatives_.set_values(0);
}


void Particle::show(std::ostream& out) const
{
  const std::string inset("  ");
  out << std::endl;
  out << "Particle: " << get_index() << std::endl;
  if (is_active_) {
    out << inset << inset << "active";
  } else {
    out << inset << inset << "dead";
  }
  out << std::endl;

  if (get_model() != NULL) {
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
