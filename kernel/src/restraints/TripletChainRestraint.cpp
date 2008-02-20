/**
 *  \file TripletChainRestraint.cpp \brief Triplet restraint.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/restraints/TripletChainRestraint.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/Particle.h"
#include "IMP/Model.h"
#include "IMP/log.h"
#include "IMP/Vector3D.h"
#include "IMP/TripletScore.h"

#include <cmath>

namespace IMP
{

TripletChainRestraint::TripletChainRestraint(TripletScore* ts)
{
  ts_ = std::auto_ptr<TripletScore>(ts);
  clear_chains();
}

void TripletChainRestraint::add_chain(const Particles &ps)
{
  if (ps.size() <3) {
    IMP_WARN("Adding a chain of length 2 or less to the AnglesRestraint"
             << " doesn't accomplish anything."<< std::endl);
  } else {
    Restraint::add_particles(ps);
    chain_splits_.back()= Restraint::number_of_particles();
    chain_splits_.push_back(Restraint::number_of_particles());
  }
}

Float TripletChainRestraint::evaluate(DerivativeAccumulator *accum)
{
  int cur_break=0;
  unsigned int i=2;
  float score=0;
  while (i < Restraint::number_of_particles()) {
    /*IMP_LOG(VERBOSE, "Chain eval on " 
            << Restraint::get_particle(i-2)->get_index()
            << Restraint::get_particle(i-1)->get_index()
            << Restraint::get_particle(i)->get_index()
            << " split is " << chain_splits_[cur_break]
            << std::endl);*/
    score += ts_->evaluate(Restraint::get_particle(i-2),
                           Restraint::get_particle(i-1),
                           Restraint::get_particle(i),
                           accum);
    if (chain_splits_[cur_break] == i) {
      i+=3;
      ++cur_break;
    } else {
      ++i;
    }
  } 
  return score;
}

void TripletChainRestraint::clear_chains() {
  Restraint::clear_particles();
  chain_splits_.clear();
  chain_splits_.push_back(0);
}


void TripletChainRestraint::show(std::ostream& out) const
{
  if (get_is_active()) {
    out << "Triplet chain restraint (active):" << std::endl;
  } else {
    out << "Triplet chain restraint (inactive):" << std::endl;
  }

  out << "  version: " << version() << "  ";
  out << "  last_modified_by: " << last_modified_by() << std::endl;
  out << "  " << chain_splits_.size()-1 << " chains" << std::endl;
  ts_->show(out);
  out << std::endl;
}

}  // namespace IMP
