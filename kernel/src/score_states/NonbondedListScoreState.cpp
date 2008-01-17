/**
 *  \file NonbondedListScoreState.cpp
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/score_states/NonbondedListScoreState.h"

namespace IMP
{

NonbondedListScoreState::NonbondedListScoreState(const Particles &ps,
                                                 float dist_cutoff) :
    dist_cutoff_(dist_cutoff)
{
  set_particles(ps);
}

void NonbondedListScoreState::rescan()
{
  nbl_.clear();
  for (unsigned int i = 0; i < ps_.size(); ++i) {
    Particle *pi = ps_[i];
    for (unsigned int j = 0; j < i; ++j) {
      Particle *pj = ps_[j];
      bool found = false;
      for (BondedListScoreStateIterator bli = bonded_list_score_states_begin();
           bli != bonded_list_score_states_end(); ++bli) {
        if ((*bli)->are_bonded(pi, pj)) {
          found = true;
          break;
        }
      }
      if (!found) {
        nbl_.push_back(std::make_pair(pi, pj));
      }
    }
  }
}

void NonbondedListScoreState::set_particles(const Particles &ps)
{
  ps_ = ps;
  rescan();
}

void NonbondedListScoreState::update()
{
  IMP_LOG(VERBOSE, "Updating non-bonded list" << std::endl);
  rescan();
}

void NonbondedListScoreState::show(std::ostream &out) const
{
  out << "NonbondedList" << std::endl;
}

IMP_CONTAINER_IMPL(NonbondedListScoreState, BondedListScoreState,
                   bonded_list_score_state, BondedListIndex,);

} // namespace IMP
