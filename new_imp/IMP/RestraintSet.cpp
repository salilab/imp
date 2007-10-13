/*
 *  RestraintSet.cpp
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */


#include "RestraintSet.h"
#include "ModelData.h"
#include "log.h"
#include "restraints/Restraint.h"

namespace IMP
{



//######### Restraint Set #########
// Used to hold a set of related restraints


/**
  Constructor
 */

RestraintSet::RestraintSet (const std::string name)
{
  IMP_LOG(VERBOSE, "Restraint set constructed");

  restraints_.resize(0);

  name_ = name;
  is_active_ = true;  // active by default
}


/**
Destructor
 */

RestraintSet::~RestraintSet ()
{
  IMP_LOG(VERBOSE,"Delete RestraintSet: beware of early Python calls to destructor.");
}


/**
Give accesss to model particle data.

\param[in] model_data All particle data in the model.
*/

void RestraintSet::set_model_data(ModelData* model_data)
{
  model_data_ = model_data;
}

/** Get name of this restraint set. */
std::string RestraintSet::name(void) const {
  return name_;
}

/** Set name of this restraint set. */
void RestraintSet::set_name(const std::string name) {
  name_ = name;
}

/**
  Add restraint to the restraint set.

  \param[in] restraint The restraint to add to the restraint set.
 */

void RestraintSet::add_restraint(Restraint* restraint)
{
  restraints_.push_back(restraint);
}


/**
  Add restraint set to the restraint set.

  \param[in] restraint_set The restraint set to add to this restraint set.
 */

void RestraintSet::add_restraint_set(RestraintSet* restraint_set)
{
  restraint_set->set_model_data(model_data_);
  restraint_sets_.push_back(restraint_set);
}


/**
  Calculate the score for this distance restraint based on the current
  state of the model.

 \param[in] calc_deriv If true, partial first derivatives should be calculated.
 */

Float RestraintSet::evaluate(bool calc_deriv)
{
  Float score;

  score = (Float) 0.0;
  for (size_t i=0; i < restraints_.size(); i++) {
    if (restraints_[i]->is_active())
      score += restraints_[i]->evaluate(calc_deriv);
  }

  for (size_t i=0; i < restraint_sets_.size(); i++) {
    if (restraint_sets_[i]->is_active())
      score += restraint_sets_[i]->evaluate(calc_deriv);
  }

  return score;
}


/**
  One or more particles may have been activated or deactivated. Check each restraint
to see if it changes its active status.
*/

void RestraintSet::check_particles_active(void)
{
  for (size_t i=0; i < restraints_.size(); i++) {
    restraints_[i]->check_particles_active();
  }

  for (size_t i=0; i < restraint_sets_.size(); i++) {
    restraint_sets_[i]->check_particles_active();
  }
}


/**
  Change the flag of restraint set.

 \param[in] is_active True is restraint set should be evaluated.
 */

void RestraintSet::set_is_active (const bool is_active)
{
  is_active_ = is_active;
  if (is_active_) {
    IMP_LOG(VERBOSE, name_ << " was set to active.");
  } else {
    IMP_LOG(VERBOSE, name_ << " was set to inactive.");
  }
}

/**
  Indicate if restraint set is currently active.

 \return true if restraint set is active.
 */

bool RestraintSet::is_active (void) const
{
  return(is_active_);
}

/**
  Show the current restraint.

 \param[in] out Stream to send restraint description to.
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



