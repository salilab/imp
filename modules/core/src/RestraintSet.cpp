/**
 *  \file RestraintSet.cpp   \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */


#include <IMP/core/RestraintSet.h>

#include <IMP/log.h>

#include <memory>
#include <utility>

IMPCORE_BEGIN_NAMESPACE

RestraintSet::RestraintSet(const std::string& name)
    : weight_(1.0)
{
  name_=name;
}



IMP_LIST_IMPL(RestraintSet, Restraint, restraint, Restraint*,
              obj->set_model(get_model());,,obj->set_model(NULL););


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

ParticlesList RestraintSet::get_interacting_particles() const
{
  ParticlesList ret;
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    ParticlesList c= (*it)->get_interacting_particles();
    ret.insert(ret.end(), c.begin(), c.end());
  }
  return ret;
}

void RestraintSet::show(std::ostream& out) const
{
  out << "restraint set " << name_ << ":..." << std::endl;
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    (*it)->show(out);
  }
  out << "... end restraint set " << name_ << std::endl;
}

IMPCORE_END_NAMESPACE
