/**
 *  \file BipartiteNonbondedListScoreState.cpp  
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/score_states/BipartiteNonbondedListScoreState.h"
#include "IMP/decorators/XYZDecorator.h"

namespace IMP
{

BipartiteNonbondedListScoreState
::BipartiteNonbondedListScoreState(const Particles &ps0,
                                   const Particles &ps1):
  NonbondedListScoreState(ps0)
{
  set_particles(ps0, ps1);
}

BipartiteNonbondedListScoreState::~BipartiteNonbondedListScoreState()
{
}

void BipartiteNonbondedListScoreState::rescan()
{
  nbl_.clear();
  for (unsigned int i = 0; i< ps_.size(); ++i) {
    Particle *pi= ps_[i];
    for (unsigned int j = 0; j < P::ps_.size(); ++j) {
      Particle *pj = P::ps_[j];
      P::add_if_nonbonded(pi,pj);
    }
  }
}

void BipartiteNonbondedListScoreState::set_particles(const Particles &ps0,
                                                     const Particles &ps1) 
{
  ps_=ps0;
  P::ps_=ps1;
  Particles aps(ps0);
  aps.insert(aps.end(), ps1.begin(), ps1.end());

  P::audit_particles(aps);
  P::propagate_set_particles(aps);

  rescan();
}

void BipartiteNonbondedListScoreState::update()
{
  P::propagate_update();
  rescan();
}

void BipartiteNonbondedListScoreState::show(std::ostream &out) const
{
  out << "BipartiteNonbondedList" << std::endl;
}

} // namespace IMP
