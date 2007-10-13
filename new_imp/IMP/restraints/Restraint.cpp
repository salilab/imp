/*
 *  Restraint.cpp
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "../Particle.h"
#include "../Model.h"
#include "../log.h"
#include "Restraint.h"

namespace IMP
{

//######### Restraint #########
// Abstract class for representing restraints

/**
  Constructor
 */

Restraint::Restraint (std::string name): name_(name)
{
  IMP_LOG(VERBOSE, "Restraint constructed");
  is_active_ = true; // active by default
  are_particles_active_ = true; // active by default
}


/**
Destructor
 */

Restraint::~Restraint ()
{
  IMP_LOG(VERBOSE, "Restraint deleted");
}



/**
Give accesss to model particle data.

\param[in] model_data All particle data in the model.
*/

void Restraint::set_model_data(ModelData* model_data)
{
  model_data_ = model_data;
}


/**
  Set whether the restraint is active. I.e. if it should be
evaluated.

 \param[in] is_active If true, the restraint is active.
 */

void Restraint::set_is_active(const bool active)
{
  is_active_ = active;
}


/**
  Get whether the restraint is active. I.e. if it should be
evaluated.

 \return true it the restraint is active.
 */

bool Restraint::is_active(void) const
{
  return is_active_  && are_particles_active_;
}


/**
  Show the current restraint.

 \param[in] out Stream to send restraint description to.
 */

void Restraint::show(std::ostream& out) const
{
  if (is_active()) {
    out << "unknown restraint (active):" << std::endl;
  } else {
    out << "unknown restraint (inactive):" << std::endl;
  }

  out << "version: " << version() << std::endl;
  out << "last_modified_by: " << last_modified_by() << std::endl;
}


/**
  Check if all necessary particles are still active. If not, inactivate self.
 */

void Restraint::check_particles_active(void)
{
  are_particles_active_ = true;
  for (size_t i = 0; i < particles_.size(); i++) {
    if (!particles_[i]->is_active()) {
      are_particles_active_ = false;
      return;
    }
  }
}

}  // namespace IMP
