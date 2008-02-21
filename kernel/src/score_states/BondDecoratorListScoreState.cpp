/**
 *  \file BondDecoratorListScoreState.cpp
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/score_states/BondDecoratorListScoreState.h"

#include <algorithm>

namespace IMP 
{

BondDecoratorListScoreState::BondDecoratorListScoreState(const Particles &ps)
{
  set_particles(ps);
}

void BondDecoratorListScoreState::update()
{
  IMP_LOG(VERBOSE, "Updating BondDecoratorList for "
          << ps_.size() << " particles" << std::endl);
  bonds_.clear();
  for (unsigned int i=0; i< ps_.size(); ++i) {
    if (!ps_[i]->get_is_active()) continue;
    BondedDecorator di= BondedDecorator::cast(ps_[i]);
    ParticleIndex pi= ps_[i]->get_index();
    for (unsigned int j=0; j< di.get_number_of_bonds(); ++j) {
      BondedDecorator dj= di.get_bonded(j);
      if (! dj.get_particle()->get_is_active()) continue;
      if (!std::binary_search(ps_.begin(), ps_.end(), dj.get_particle())) {
        IMP_LOG(VERBOSE, "Particle " << dj.get_particle()->get_index()
                << " not found in list" << std::endl);
        continue;
      }
      if (di < dj) {
        bonds_.push_back(di.get_bond(j));
      }
    }
  }
  IMP_LOG(VERBOSE, "Found " << bonds_.size() << " bonds"<< std::endl);
}

void BondDecoratorListScoreState::set_particles(const Particles &ps)
{
  ps_=ps;
  std::sort(ps_.begin(), ps_.end());
  bonds_.clear();
}


bool BondDecoratorListScoreState::are_bonded(Particle *a, Particle *b) const
{
  try {
    BondedDecorator da= BondedDecorator::cast(a);
    BondedDecorator db= BondedDecorator::cast(b);
    return get_bond(da, db) != BondDecorator();
  } catch (...) {
    IMP_LOG(VERBOSE, "Exception thrown in are_bonded"<< std::endl);
  }
  return false;
}

} // namespace IMP
