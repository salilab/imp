/**
 *  \file RestraintSet.cpp   \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */


#include "IMP/log.h"
#include "IMP/restraints/RestraintSet.h"
#include <utility>

namespace IMP
{


//! Constructor
RestraintSet::RestraintSet(const std::string& name)
    : weight_(1.0)
{
  IMP_LOG(VERBOSE, "Restraint set constructed");
  name_=name;
}



//! Destructor
/** \todo Should reference count restraints correctly, to avoid deleting
          restraints here which live in two or more RestraintSets.
 */
RestraintSet::~RestraintSet()
{
  IMP_LOG(VERBOSE, "Delete RestraintSet");
}

IMP_CONTAINER_IMPL(RestraintSet, Restraint, restraint, RestraintIndex, 
                   obj->set_model(get_model());,,);



//! Calculate the score for this restraint for the current model state.
/** \param[in] accum If not NULL, use this object to accumulate partial first
                     derivatives.
    \return Current score.
 */
Float RestraintSet::evaluate(DerivativeAccumulator *accum)
{
  if (get_weight() == 0) return 0;
  Float score;
  typedef std::auto_ptr<DerivativeAccumulator> DAP;
  // Use a local copy of the accumulator for our sub-restraints
  DAP ouracc;
  if (accum) {
    ouracc = DAP(new DerivativeAccumulator(*accum, weight_));
  }

  score = (Float) 0.0;
  for (RestraintIterator it= restraints_begin(); it != restraints_end(); ++it) {
    if ((*it)->get_is_active()) {
      score += (*it)->evaluate(ouracc.get());
    }
  }

  return score * weight_;
}


//! Show the current restraint.
/** \param[in] out Stream to send restraint description to.
 */
void RestraintSet::show(std::ostream& out) const
{
  out << "restraint set " << name_ << ":..." << std::endl;
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    (*it)->show(out);
  }
  out << "... end restraint set " << name_ << std::endl;
}


}  // namespace IMP
