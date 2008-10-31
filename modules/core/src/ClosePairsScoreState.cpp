/**
 *  \file ClosePairsScoreState.cpp
 *  \brief Keep track of the maximumimum change of a set of attributes.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */


#include <IMP/core/ClosePairsScoreState.h>
#include <IMP/core/GridClosePairsFinder.h>
#include <IMP/core/BoxSweepClosePairsFinder.h>

#include <algorithm>
#include <sstream>

IMPCORE_BEGIN_NAMESPACE

ClosePairsScoreState::ClosePairsScoreState(ParticleContainer *pc,
                                     FilteredListParticlePairContainer* out,
                                           FloatKey rk)
{
  in_=pc;
  out_=out;
  rk_=rk;
  initialize();
}

ClosePairsScoreState::ClosePairsScoreState(ParticleContainer *pc,
                                           FloatKey rk)
{
  in_=pc;
  out_=new FilteredListParticlePairContainer();
  rk_=rk;
  initialize();
}


void ClosePairsScoreState::initialize() {
  distance_=0;
  slack_=1;
  if (BoxSweepClosePairsFinder::get_is_implemented()) {
    set_close_pairs_finder(new BoxSweepClosePairsFinder());
  } else {
    set_close_pairs_finder(new GridClosePairsFinder());
  }
}

void ClosePairsScoreState::set_distance(Float d) {
  distance_=d;
}

void ClosePairsScoreState::set_slack(Float d) {
  slack_=d;
}

void ClosePairsScoreState::set_particle_container(ParticleContainer *pc) {
  // needs to be first for the case of assigning the pc that is already there
  in_=pc;
  xyzc_->set_particle_container(in_);
  if (rc_) rc_->set_particle_container(in_);
}

void ClosePairsScoreState::set_close_pairs_finder(ClosePairsFinder *f) {
  f_=f;
}

void ClosePairsScoreState::set_radius_key(FloatKey k) {
  rk_=k;
  rc_=NULL;
  xyzc_=NULL;
}

ClosePairsScoreState::~ClosePairsScoreState() {}

void ClosePairsScoreState::do_before_evaluate()
{
  IMP_CHECK_OBJECT(in_);
  IMP_CHECK_OBJECT(out_);
  IMP_CHECK_OBJECT(f_);
  if (!xyzc_) {
    //std::cout << "Virgin ss" << std::endl;
    xyzc_ =new MaximumChangeScoreState(in_, XYZDecorator::get_xyz_keys());
    if (rk_ != FloatKey()) {
      rc_= new MaximumChangeScoreState(in_, FloatKeys(1, rk_));
    }
    //std::cout << "adding pairs" << std::endl;
    out_->clear_particle_pairs();
    f_->add_close_pairs(in_, distance_+slack_,
                        rk_, out_);
    //std::cout << "done"<< std::endl;
    return;
  } else {
    xyzc_->before_evaluate(ScoreState::get_before_evaluate_iteration());
    if (rc_){
      rc_->before_evaluate(ScoreState::get_before_evaluate_iteration());
    }
    Float delta= xyzc_->get_maximum_change()
      + (rc_ ? rc_->get_maximum_change(): 0);
    if (delta*2 > slack_) {
      out_->clear_particle_pairs();
      f_->add_close_pairs(in_, distance_+slack_,
                          rk_, out_);
      xyzc_->reset();
      if (rc_) {
        rc_->reset();
      }
    }
  }
}


void ClosePairsScoreState::show(std::ostream &out) const
{
  out << "ClosePairsScoreState" << std::endl;
}

IMPCORE_END_NAMESPACE
