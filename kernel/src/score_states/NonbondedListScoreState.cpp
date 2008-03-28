/**
 *  \file NonbondedListScoreState.cpp
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/score_states/NonbondedListScoreState.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/internal/Grid3D.h"
#include "IMP/score_states/MaxChangeScoreState.h"

namespace IMP
{

NonbondedListScoreState::NonbondedListScoreState(FloatKey rk): rk_(rk)
{
  last_cutoff_=-1;
}





void NonbondedListScoreState::propagate_particles(const Particles&ps)
{
  clear_nbl();
  for (BondedListScoreStateIterator bli= bonded_lists_begin();
       bli != bonded_lists_end(); ++bli) {
    (*bli)->set_particles(ps);
  }
}

void NonbondedListScoreState::update()
{
  IMP_LOG(VERBOSE, "Updating non-bonded list" << std::endl);
  for (BondedListScoreStateIterator bli= bonded_lists_begin();
       bli != bonded_lists_end(); ++bli) {
    (*bli)->update();
  }
}

void NonbondedListScoreState::show(std::ostream &out) const
{
  out << "NonbondedList" << std::endl;
}

IMP_CONTAINER_IMPL(NonbondedListScoreState, BondedListScoreState,
                   bonded_list, BondedListIndex, {
                     if (0) std::cout <<*obj;
                   });
} // namespace IMP
