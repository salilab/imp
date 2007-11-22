/**
 *  \file RestraintSet.cpp   \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */


#include "IMP/ModelData.h"
#include "IMP/log.h"
#include "IMP/restraints/RestraintSet.h"
#include "../mystdexcept.h"

namespace IMP
{


//! Constructor
RestraintSet::RestraintSet(const std::string& name)
    : Restraint(name), weight_(1.0)
{
  IMP_LOG(VERBOSE, "Restraint set constructed");
}


//! Destructor
/** \todo Should reference count restraints correctly, to avoid deleting
          restraints here which live in two or more RestraintSets.
 */
RestraintSet::~RestraintSet()
{
  IMP_LOG(VERBOSE, "Delete RestraintSet");
  for (unsigned int i=0; i< restraints_.size(); ++i) {
    restraints_[i]->set_model(NULL);
    delete restraints_[i];
  }

}


//! Add restraint to the restraint set.
/** \param[in] restraint The restraint to add to the restraint set.
    \return the index of the newly-added restraint in the restraint set.
 */
RestraintSet::RestraintIndex RestraintSet::add_restraint(Restraint* restraint)
{
  restraints_.push_back(restraint);
  restraint->set_model(get_model());
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
/** \param[in] accum If not NULL, use this object to accumulate partial first
                     derivatives.
    \return Current score.
 */
Float RestraintSet::evaluate(DerivativeAccumulator *accum)
{
  Float score;

  // Use a local copy of the accumulator for our sub-restraints
  DerivativeAccumulator *ouracc = NULL;
  if (accum) {
    ouracc = new DerivativeAccumulator(*accum, weight_);
  }

  score = (Float) 0.0;
  try {
    for (size_t i=0; i < restraints_.size(); i++) {
      if (restraints_[i]->get_is_active())
        score += restraints_[i]->evaluate(ouracc);
    }
  } catch (...) {
    delete ouracc;
    throw;
  }
  delete ouracc;

  return score * weight_;
}


//! Called when at least one particle has been inactivated.
/** Check each restraint to see if it changes its active status.
 */
void RestraintSet::check_particles_active()
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
  out << "restraint set " << get_name() << ":" << std::endl;
  for (size_t i=0; i < restraints_.size(); i++) {
    restraints_[i]->show(out);
  }
  out << "... end restraint set " << get_name() << std::endl;
}


}  // namespace IMP
