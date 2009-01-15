/**
 *  \file TripletChainRestraint.cpp \brief Triplet restraint.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/TripletChainRestraint.h>
#include <IMP/core/XYZDecorator.h>

#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/log.h>
#include <IMP/Vector3D.h>
#include <IMP/TripletScore.h>

#include <cmath>

IMPCORE_BEGIN_NAMESPACE

TripletChainRestraint::TripletChainRestraint(TripletScore* ts)
{
  ts_ = ts;
  clear_chains();
}

void TripletChainRestraint::add_chain(const Particles &ps)
{
  if (ps.size() <3) {
    IMP_WARN("Adding a chain of length 2 or less to the AnglesRestraint"
             << " doesn't accomplish anything."<< std::endl);
  } else {
    add_particles(ps);
    chain_splits_.back()= get_number_of_particles();
    chain_splits_.push_back(get_number_of_particles());
  }
}

IMP_LIST_IMPL(TripletChainRestraint, Particle, particle, Particle*,,)

Float TripletChainRestraint::evaluate(DerivativeAccumulator *accum)
{
  int cur_break=0;
  unsigned int i=2;
  float score=0;
  while (i < get_number_of_particles()) {
    /*IMP_LOG(VERBOSE, "Chain eval on "
            << get_particle(i-2)->get_index()
            << get_particle(i-1)->get_index()
            << get_particle(i)->get_index()
            << " split is " << chain_splits_[cur_break]
            << std::endl);*/
    score += ts_->evaluate(get_particle(i-2),
                           get_particle(i-1),
                           get_particle(i),
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

void TripletChainRestraint::clear_chains()
{
  clear_particles();
  chain_splits_.clear();
  chain_splits_.push_back(0);
}

ParticlesList TripletChainRestraint::get_interacting_particles() const
{
  ParticlesList ret(get_number_of_particles()-2);
  for (unsigned int i=2; i< get_number_of_particles(); ++i) {
    ret[i-2]= Particles(3);
    ret[i-2][0]= get_particle(i);
    ret[i-2][1]= get_particle(i-1);
    ret[i-2][2]= get_particle(i-2);
  }
  return ret;
}

void TripletChainRestraint::show(std::ostream& out) const
{
  if (get_is_active()) {
    out << "Triplet chain restraint (active):" << std::endl;
  } else {
    out << "Triplet chain restraint (inactive):" << std::endl;
  }

  get_version_info().show(out);
  out << "  " << chain_splits_.size()-1 << " chains" << std::endl;
  ts_->show(out);
  out << std::endl;
}

IMPCORE_END_NAMESPACE
