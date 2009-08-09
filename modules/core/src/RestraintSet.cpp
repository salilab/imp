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
  : Restraint(name), weight_(1.0)
{
}



IMP_LIST_IMPL(RestraintSet, Restraint, restraint, Restraint*,
              Restraints,
              if (get_is_part_of_model()) {
                obj->set_model(get_model());
              },,obj->set_model(NULL););


Float RestraintSet::evaluate(DerivativeAccumulator *accum)
{
  if (get_weight() == 0) return 0;
  IMP_OBJECT_LOG;
  Float score;
  typedef std::auto_ptr<DerivativeAccumulator> DAP;
  // Use a local copy of the accumulator for our sub-restraints
  DAP ouracc;
  if (accum) {
    ouracc = DAP(new DerivativeAccumulator(*accum, weight_));
  }

  score = (Float) 0.0;
  for (RestraintIterator it= restraints_begin(); it != restraints_end(); ++it) {
    IMP_LOG(TERSE, "Evaluate restraint " << (*it)->get_name() << std::endl);
    double tscore= (*it)->evaluate(ouracc.get());
    score+=tscore;
    IMP_LOG(TERSE, "Restraint score is " << tscore << std::endl);
  }

  return score * weight_;
}

void RestraintSet::set_model(Model *m) {
  Restraint::set_model(m);
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    (*it)->set_model(m);
  }
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
  out << "restraint set " << get_name() << ":..." << std::endl;
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    (*it)->show(out);
  }
  out << "... end restraint set " << get_name() << std::endl;
}

IMPCORE_END_NAMESPACE
