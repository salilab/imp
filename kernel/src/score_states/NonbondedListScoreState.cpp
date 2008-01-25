/**
 *  \file NonbondedListScoreState.cpp
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/score_states/NonbondedListScoreState.h"
#include "IMP/decorators/XYZDecorator.h"

namespace IMP
{

NonbondedListScoreState::NonbondedListScoreState(const Particles &ps)
{
  set_particles(ps);
}

NonbondedListScoreState::~NonbondedListScoreState()
{
  IMP_CONTAINER_DELETE(BondedListScoreState, bonded_list);
}

void NonbondedListScoreState::audit_particles(const Particles &ps) const
{
  for (unsigned int i=0; i< ps.size(); ++i) {
    try {
      XYZDecorator d= XYZDecorator::cast(ps[i]);
    } catch (...) {
      IMP_WARN("Particle " << ps[i]->get_index() 
               << " does not have x,y,z coordinates " 
               << " but was passed to the NonbondedListScoreState.\n");
    }
  }
}

void NonbondedListScoreState::propagate_set_particles(const Particles &aps) 
{
  for (BondedListScoreStateIterator bli= bonded_lists_begin();
       bli != bonded_lists_end(); ++bli) {
    (*bli)->set_particles(aps);
  }
}

void NonbondedListScoreState::propagate_update() 
{
  for (BondedListScoreStateIterator bli= bonded_lists_begin();
       bli != bonded_lists_end(); ++bli) {
    (*bli)->update();
  }
}

void NonbondedListScoreState::add_if_nonbonded(Particle *a, Particle *b)
{
  bool found=false;
  for (BondedListIterator bli= bonded_lists_begin();
       bli != bonded_lists_end(); ++bli) {
    if ((*bli)->are_bonded(a, b)) {
      found = true;
      break;
    }
  }
  if (!found) {
    nbl_.push_back(std::make_pair(a, b));
  }
}


void NonbondedListScoreState::rescan()
{
  nbl_.clear();
  for (unsigned int i = 0; i < ps_.size(); ++i) {
    Particle *pi = ps_[i];
    for (unsigned int j = 0; j < i; ++j) {
      Particle *pj = ps_[j];
      add_if_nonbonded(pi, pj);
    }
  }
}

void NonbondedListScoreState::set_particles(const Particles &ps)
{
  ps_ = ps;
  audit_particles(ps_);
  propagate_set_particles(ps);
  rescan();
}

void NonbondedListScoreState::update()
{
  IMP_LOG(VERBOSE, "Updating non-bonded list" << std::endl);
  propagate_update();
  rescan();
}

void NonbondedListScoreState::show(std::ostream &out) const
{
  out << "NonbondedList" << std::endl;
}

IMP_CONTAINER_IMPL(NonbondedListScoreState, BondedListScoreState,
                   bonded_list, BondedListIndex, );
} // namespace IMP
