/**
 *  \file Restraint.cpp   \brief Abstract base class for all restraints.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/Particle.h"
#include "IMP/Model.h"
#include "IMP/log.h"
#include "IMP/restraints/Restraint.h"

namespace IMP
{

//! Constructor
Restraint::Restraint(std::string name): name_(name)
{
  model_=NULL;
  IMP_LOG(VERBOSE, "Restraint constructed");
  is_active_ = true; // active by default
  are_particles_active_ = true; // active by default
}


//! Destructor
Restraint::~Restraint()
{
  IMP_LOG(VERBOSE, "Restraint deleted");
 IMP_assert(get_model() == NULL,
             "Attemping to delete restraint while it is still in a model.\n"
            << " This probably reflects premature destruction"
            << " of python references.\n");
}


//! Set whether the restraint is active i.e. if it should be evaluated.
/** \param[in] is_active If true, the restraint is active.
 */
void Restraint::set_is_active(const bool is_active)
{
  is_active_ = is_active;
}


//! Get whether the restraint is active. i.e. if it should be evaluated.
/** \return true if the restraint is active.
 */
bool Restraint::get_is_active() const
{
  return is_active_ && are_particles_active_;
}


//! Show the current restraint.
/** \param[in] out Stream to send restraint description to.
 */
void Restraint::show(std::ostream& out) const
{
  if (get_is_active()) {
    out << "unknown restraint (active):" << std::endl;
  } else {
    out << "unknown restraint (inactive):" << std::endl;
  }

  out << "version: " << version() << std::endl;
  out << "last_modified_by: " << last_modified_by() << std::endl;
}


//! Check if all necessary particles are still active.
/** If not, inactivate self. Called when at least one model particle
    has been inactivated.
 */
void Restraint::check_particles_active()
{
  are_particles_active_ = true;
  for (size_t i = 0; i < particles_.size(); i++) {
    if (!particles_[i]->get_is_active()) {
      are_particles_active_ = false;
      return;
    }
  }
}

}  // namespace IMP
