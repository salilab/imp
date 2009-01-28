/**
 *  \file BondDecoratorListScoreState.cpp
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include <IMP/deprecation.h>
#include <IMP/core/BondDecoratorListScoreState.h>
#include <IMP/deprecation.h>

#include <algorithm>

IMPCORE_BEGIN_NAMESPACE

BondDecoratorListScoreState::BondDecoratorListScoreState(const Particles &ps)
{
  set_particles(ps);
  IMP_DEPRECATED(BondDecoratorListScoreState, BondDecoratorPairContainer);
}

void BondDecoratorListScoreState::do_before_evaluate()
{
  IMP_LOG(TERSE, "Updating BondDecoratorList for "
          << ps_.size() << " particles" << std::endl);
  bonds_.clear();
  for (unsigned int i=0; i< ps_.size(); ++i) {
    if (!ps_[i]->get_is_active()) continue;
    BondedDecorator di(ps_[i]);
    for (unsigned int j=0; j< di.get_number_of_bonds(); ++j) {
      BondedDecorator dj= di.get_bonded(j);
      if (! dj.get_particle()->get_is_active()) continue;
      if (!std::binary_search(ps_.begin(), ps_.end(), dj.get_particle())) {
        IMP_LOG(VERBOSE, "Particle " << dj.get_particle()->get_name()
                << " not found in list" << std::endl);
        continue;
      }
      if (di < dj) {
        IMP_LOG(VERBOSE, "Found bond " << di.get_bond(j) << std::endl);
        bonds_.push_back(di.get_bond(j));
      }
    }
  }
  IMP_LOG(TERSE, "Found " << bonds_.size() << " bonds"<< std::endl);
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

IMPCORE_END_NAMESPACE
