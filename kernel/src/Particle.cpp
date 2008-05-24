/**
 *  \file Particle.cpp   \brief Classes to handle individual model particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Model.h"


namespace IMP
{

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
            ValueException("Cannot transfer particles directly"));
  model_ = md;
  pi_ = pi;
  IMP_check(model_==NULL || model_->get_particle(pi_)== this,
            "Set_model called with inconsistent data",
            ValueException("Cannot transfer particles directly"));
}

void Particle::set_is_active(const bool is_active)
{
  is_active_ = is_active;
}


void Particle::zero_derivatives()
{
  for (FloatKeyIterator it= float_keys_begin(); it != float_keys_end(); ++it) {
    derivatives_.set_value(*it, 0);
  }
}


void Particle::show(std::ostream& out) const
{
  const char* inset = "  ";
  out << std::endl;
  out << "--" << get_index() << "--" << std::endl;
  if (is_active_) {
    out << inset << inset << "active";
  } else {
    out << inset << inset << "dead";
  }
  out << std::endl;

  if (get_model() != NULL) {
    out << inset << inset << "float attributes:" << std::endl;
    floats_.show(out, "    ");

    out << inset << inset << "int attributes:" << std::endl;
    ints_.show(out, "    ");

    out << inset << inset << "string attributes:" << std::endl;
    strings_.show(out, "    ");

    out << inset << inset << "particle attributes:" << std::endl;
    particles_.show(out, "    ");

  }
}


}  // namespace IMP
