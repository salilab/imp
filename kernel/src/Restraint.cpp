/**
 *  \file Restraint.cpp   \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/Particle.h"
#include "IMP/Model.h"
#include "IMP/log.h"
#include "IMP/Restraint.h"

namespace IMP
{

Restraint::Restraint()
{
  model_ = NULL;
  is_active_ = true; // active by default
  are_particles_active_ = true; // active by default
}


//! Destructor
Restraint::~Restraint()
{
}


void Restraint::set_is_active(const bool is_active)
{
  is_active_ = is_active;
}


bool Restraint::get_is_active() const
{
  return is_active_ && are_particles_active_;
}


void Restraint::set_model(Model* model)
{
  IMP_assert(model== NULL || particles_.empty() 
             || model == particles_[0]->get_model(),
             "Model* different from Particle Model*");
  model_=model;
}

void Restraint::show(std::ostream& out) const
{
  if (get_is_active()) {
    out << "unknown restraint (active):" << std::endl;
  } else {
    out << "unknown restraint (inactive):" << std::endl;
  }

  out << "  version: " << version() << std::endl;
  out << "  last_modified_by: " << last_modified_by() << std::endl;
}


void Restraint::check_particles_active()
{
  IMP_assert(get_model() != NULL,
             "Add Restraint to Model before calling check_particles_active");
  are_particles_active_ = true;
  for (size_t i = 0; i < particles_.size(); i++) {
    if (!get_particle(i)->get_is_active()) {
      are_particles_active_ = false;
      return;
    }
  }
}

}  // namespace IMP
