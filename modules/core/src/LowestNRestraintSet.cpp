/**
 *  \file LowestNRestraintSet.cpp
 *  \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */


#include "IMP/core/LowestNRestraintSet.h"
#include "IMP/core/internal/MinimalSet.h"

#include <IMP/log.h>

#include <memory>
#include <utility>

IMPCORE_BEGIN_NAMESPACE

LowestNRestraintSet::LowestNRestraintSet(unsigned int n): n_(n)
{
}



LowestNRestraintSet::~LowestNRestraintSet()
{
}

IMP_CONTAINER_IMPL(LowestNRestraintSet, Restraint, restraint, RestraintIndex,
                   obj->set_model(get_model());,,);



namespace {
typedef internal::MinimalSet<float,
                             LowestNRestraintSet::RestraintConstIterator> MS;
template <class It>
MS find_minimal_set(It b, It e, unsigned int n) {
  IMP_LOG(TERSE, "Finding lowest " << n << " of "
          << std::distance(b,e) << std::endl);
  MS bestn(n);
  for (It it= b; it != e; ++it) {
    float score= (*it)->evaluate(NULL);

    if (bestn.can_insert(score)) {
      bestn.insert(score, it);
    }
  }
  return bestn;
}
}

Float LowestNRestraintSet::evaluate(DerivativeAccumulator *accum)
{
  /* hack for visual studio since it does not seem to be able to
     convert the nonconst iterator to a const one */
  const LowestNRestraintSet *const_this
    = static_cast<const LowestNRestraintSet*>(this);
  MS bestn= find_minimal_set(const_this->restraints_begin(),
                             const_this->restraints_end(), n_);

  float score=0;
  for (unsigned int i=0; i< bestn.size(); ++i) {
    if (accum) {
      (*bestn[i].second)->evaluate(accum);
    }
    score+= bestn[i].first;
  }

  return score;
}

ParticlesList LowestNRestraintSet::get_interacting_particles() const
{
  MS bestn= find_minimal_set(restraints_begin(), restraints_end(), n_);
  ParticlesList ret;
  for (unsigned int i=0; i< bestn.size(); ++i) {
    ParticlesList c= (*bestn[i].second)->get_interacting_particles();
    ret.insert(ret.end(), c.begin(), c.end());
  }

  return ret;
}

void LowestNRestraintSet::show(std::ostream& out) const
{
  out << "lowest n restraint set:..." << std::endl;
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    (*it)->show(out);
  }
  out << "... end restraint set "<< std::endl;
}

IMPCORE_END_NAMESPACE
