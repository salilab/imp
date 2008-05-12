/**
 *  \file ChainBondedListScoreState.cpp
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/score_states/ChainBondedListScoreState.h"

#include <algorithm>
#include <sstream>

namespace IMP 
{

ChainBondedListScoreState::ChainBondedListScoreState()
{
  std::ostringstream oss;
  oss << "ChainBLSS " << this;
  cik_ = IntKey(oss.str().c_str());
  next_index_ = 0;
}

void ChainBondedListScoreState::update()
{
  IMP_LOG(TERSE, "Updating ChainBondedList assumed to be static" << std::endl);
}

void ChainBondedListScoreState::clear_chains()
{
  for (unsigned int i = 0; i < chains_.size(); ++i) {
    for (unsigned int j = 0; j < chains_[i].size(); ++j) {
      chains_[i][j]->set_value(cik_, -1);
    }
  }
  chains_.clear();
  next_index_=0;
}

void ChainBondedListScoreState::add_chain(const Particles &ps)
{
  chains_.push_back(internal::Vector<Particle*>(ps.begin(), ps.end()));
  for (unsigned int i = 0; i < chains_.back().size(); ++i) {
    Particle *p = chains_.back()[i];
    if (p->has_attribute(cik_)) {
      p->set_value(cik_, next_index_);
    } else {
      p->add_attribute(cik_, next_index_);
    }
    ++next_index_;
  }
  ++next_index_;
}


bool ChainBondedListScoreState::are_bonded(Particle *a, Particle *b) const
{
  if (!a->has_attribute(cik_) || !b->has_attribute(cik_)) return false;
  int ia= a->get_value(cik_);
  int ib= b->get_value(cik_);
  return std::abs(ia-ib) == 1;
}

} // namespace IMP
