/**
 *  \file RestraintSet.cpp   \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */


#include "RestraintSet.h"
#include "../ModelData.h"
#include "../log.h"
#include "../mystdexcept.h"

namespace IMP
{


//! Constructor
RestraintSet::RestraintSet(const std::string& name): Restraint(name)
{
  IMP_LOG(VERBOSE, "Restraint set constructed");
}


//! Destructor
RestraintSet::~RestraintSet()
{
  IMP_LOG(VERBOSE,"Delete RestraintSet: beware of early Python calls to destructor.");
}


//! Add restraint to the restraint set.
/** \param[in] restraint The restraint to add to the restraint set.
    \return the index of the newly-added restraint in the restraint set.
 */
RestraintSet::RestraintIndex RestraintSet::add_restraint(Restraint* restraint)
{
  restraints_.push_back(restraint);
  return restraints_.size()-1;
}


//! Access a restraint in the restraint set.
/** \param[in] i The RestraintIndex of the restraint to retrieve.
    \exception std::out_of_range the index is out of range.
    \return Pointer to the Restraint.
 */
Restraint* RestraintSet::get_restraint(RestraintIndex i) const
{
  IMP_check(static_cast<unsigned int>(i) < restraints_.size(),
	    "Invalid restraint requested",
	    std::out_of_range("Invalid restraint"));
  return restraints_[i];
}

//! Calculate the score for this restraint for the current model state.
/** \param[in] calc_deriv If true, partial first derivatives should
                          be calculated.
    \return Current score.
 */
Float RestraintSet::evaluate(bool calc_deriv)
{
  Float score;

  score = (Float) 0.0;
  for (size_t i=0; i < restraints_.size(); i++) {
    if (restraints_[i]->is_active())
      score += restraints_[i]->evaluate(calc_deriv);
  }

  return score;
}


//! Called when at least one particle has been inactivated.
/** Check each restraint to see if it changes its active status.
 */
void RestraintSet::check_particles_active(void)
{
  for (size_t i=0; i < restraints_.size(); i++) {
    restraints_[i]->check_particles_active();
  }
}



//! Show the current restraint.
/** \param[in] out Stream to send restraint description to.
 */
void RestraintSet::show(std::ostream& out) const 
{
  out << "restraint set " << name_ << ":" << std::endl;
  for (size_t i=0; i < restraints_.size(); i++) {
    restraints_[i]->show(out);
  }
  out << "... end restraint set " << name_ << std::endl;
}


}  // namespace IMP
