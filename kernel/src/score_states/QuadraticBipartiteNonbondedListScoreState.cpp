/**
 *  \file QuadraticBipartiteNonbondedListScoreState.cpp  
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/score_states/QuadraticBipartiteNonbondedListScoreState.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/score_states/MaxChangeScoreState.h"

namespace IMP
{

QuadraticBipartiteNonbondedListScoreState
::QuadraticBipartiteNonbondedListScoreState(FloatKey rk,
                                            const Particles &ps0,
                                            const Particles &ps1): P(rk)
{
  set_particles(ps0, ps1);
}


QuadraticBipartiteNonbondedListScoreState
::QuadraticBipartiteNonbondedListScoreState(FloatKey rk): P(rk)
{
}

void QuadraticBipartiteNonbondedListScoreState::rebuild_nbl(float cut)
{
  IMP_LOG(TERSE, "Rebuilding QBNBL with lists of size " << na_
          << " and " << P::get_particles().size() -na_
          << " and cutoff " << cut << std::endl);
  for (unsigned int i=0; i< na_; ++i) {
    for (unsigned int j=na_; j < P::get_particles().size(); ++j) {
      P::handle_nbl_pair(P::get_particles()[i],
                         P::get_particles()[j],
                         cut);
    }
  }
  IMP_LOG(TERSE, "NBL has " << P::nbl_.size() << " pairs" << std::endl);
}

void QuadraticBipartiteNonbondedListScoreState
::set_particles(const Particles &ps0,
                const Particles &ps1) 
{
  IMP_LOG(TERSE, "Setting QBNBLSS particles " << ps0.size()
          << " and " << ps1.size() << std::endl);
  Particles all(ps0);
  all.insert(all.end(), ps1.begin(), ps1.end());
  P::set_particles(all);
  na_= ps0.size();
}

void QuadraticBipartiteNonbondedListScoreState::do_before_evaluate()
{
  P::do_before_evaluate();
}

void QuadraticBipartiteNonbondedListScoreState::show(std::ostream &out) const
{
  out << "QuadraticBipartiteNonbondedList" << std::endl;
}

} // namespace IMP
