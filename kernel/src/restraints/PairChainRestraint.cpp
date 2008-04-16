/**
 *  \file PairChainRestraint.cpp \brief Pair restraint.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/restraints/PairChainRestraint.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/Particle.h"
#include "IMP/Model.h"
#include "IMP/log.h"
#include "IMP/Vector3D.h"
#include "IMP/PairScore.h"

#include <cmath>

namespace IMP
{

PairChainRestraint::PairChainRestraint(PairScore* ts)
{
  ts_ = ts;
  clear_chains();
}

void PairChainRestraint::add_chain(const Particles &ps)
{
  if (ps.size() <= 1) {
    IMP_WARN("Adding a chain of length 1 or less to the PairChainsRestraint"
             << " doesn't accomplish anything."<< std::endl);
  } else {
    Restraint::add_particles(ps);
    chain_splits_.back()= Restraint::number_of_particles();
    chain_splits_.push_back(Restraint::number_of_particles());
  }
}

Float PairChainRestraint::evaluate(DerivativeAccumulator *accum)
{
  int cur_break=0;
  unsigned int i=1;
  float score=0;
  while (i < Restraint::number_of_particles()) {
    /*IMP_LOG(VERBOSE, "Chain eval on " 
            << Restraint::get_particle(i-2)->get_index()
            << Restraint::get_particle(i-1)->get_index()
            << Restraint::get_particle(i)->get_index()
            << " split is " << chain_splits_[cur_break]
            << std::endl);*/
    score += ts_->evaluate(Restraint::get_particle(i-1),
                           Restraint::get_particle(i),
                           accum);
    if (chain_splits_[cur_break] == i) {
      i+=2;
      ++cur_break;
    } else {
      ++i;
    }
  } 
  return score;
}

void PairChainRestraint::clear_chains() {
  Restraint::clear_particles();
  chain_splits_.clear();
  chain_splits_.push_back(0);
}


void PairChainRestraint::show(std::ostream& out) const
{
  if (get_is_active()) {
    out << "Pair chain restraint (active):" << std::endl;
  } else {
    out << "Pair chain restraint (inactive):" << std::endl;
  }

  get_version_info().show(out);
  out << "  " << chain_splits_.size()-1 << " chains" << std::endl;
  ts_->show(out);
  out << std::endl;
}

} // namespace IMP
