/**
 *  \file CloseBipartitePairsScoreState.cpp
 *  \brief Keep track of the maximumimum change of a set of attributes.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */


#include <IMP/core/CloseBipartitePairsScoreState.h>
#include <IMP/core/QuadraticClosePairsFinder.h>

#include <algorithm>
#include <sstream>

IMPCORE_BEGIN_NAMESPACE

CloseBipartitePairsScoreState
::CloseBipartitePairsScoreState(ParticleContainer *pc0,
                                ParticleContainer *pc1,
                                FilteredListParticlePairContainer* out,
                                           FloatKey rk)
{
  in_[0]=pc0;
  in_[1]=pc1;
  out_=out;
  rk_=rk;
  initialize();
}

CloseBipartitePairsScoreState
::CloseBipartitePairsScoreState(ParticleContainer *pc0,
                                ParticleContainer *pc1,
                                           FloatKey rk)
{
  in_[0]=pc0;
  in_[1]=pc1;
  out_=new FilteredListParticlePairContainer();
  rk_=rk;
  initialize();
}


void CloseBipartitePairsScoreState::initialize() {
  distance_=0;
  slack_=1;
  set_close_pairs_finder(new QuadraticClosePairsFinder());
}

void CloseBipartitePairsScoreState::set_distance(Float d) {
  distance_=d;
}

void CloseBipartitePairsScoreState::set_slack(Float d) {
  slack_=d;
}

void CloseBipartitePairsScoreState::clear() {
  xyzc_[0]=NULL;
  xyzc_[1]=NULL;
  rc_[0]=NULL;
  rc_[1]=NULL;
}

void CloseBipartitePairsScoreState
::set_first_particle_container(ParticleContainer *pc) {
  // needs to be first for the case of assigning the pc that is already there
  in_[0]=pc;
  clear();
}

void CloseBipartitePairsScoreState
::set_second_particle_container(ParticleContainer *pc) {
  in_[1]=pc;
  clear();
}

void CloseBipartitePairsScoreState
::set_close_pairs_finder(ClosePairsFinder *f) {
  f_=f;
}

void CloseBipartitePairsScoreState::set_radius_key(FloatKey k) {
  rk_=k;
  clear();
}

CloseBipartitePairsScoreState::~CloseBipartitePairsScoreState() {}

void CloseBipartitePairsScoreState::do_before_evaluate()
{
  IMP_CHECK_OBJECT(in_[0]);
  IMP_CHECK_OBJECT(in_[1]);
  IMP_CHECK_OBJECT(out_);
  IMP_CHECK_OBJECT(f_);
  if (!xyzc_[0]) {
    std::cout << "Virgin ss" << std::endl;
    xyzc_[0] =new MaximumChangeScoreState(in_[0], XYZDecorator::get_xyz_keys());
    xyzc_[1] =new MaximumChangeScoreState(in_[1], XYZDecorator::get_xyz_keys());
    if (rk_ != FloatKey()) {
      rc_[0]= new MaximumChangeScoreState(in_[0], FloatKeys(1, rk_));
      rc_[1]= new MaximumChangeScoreState(in_[1], FloatKeys(1, rk_));
    }
    std::cout << "adding pairs" << std::endl;
    out_->clear_particle_pairs();
    f_->add_close_pairs(in_[0], in_[1], distance_+slack_,
                        rk_, out_);
    std::cout << "done"<< std::endl;
    return;
  } else {
    xyzc_[0]->before_evaluate(ScoreState::get_before_evaluate_iteration());
    xyzc_[1]->before_evaluate(ScoreState::get_before_evaluate_iteration());
    Float delta= xyzc_[0]->get_maximum_change()
               + xyzc_[1]->get_maximum_change();
    if (rk_!= FloatKey()){
      rc_[0]->before_evaluate(ScoreState::get_before_evaluate_iteration());
      rc_[1]->before_evaluate(ScoreState::get_before_evaluate_iteration());
      delta+= rc_[0]->get_maximum_change();
      delta+= rc_[1]->get_maximum_change();
    }

    if (delta > slack_) {
      out_->clear_particle_pairs();
      f_->add_close_pairs(in_[0], in_[1], distance_+slack_,
                          rk_, out_);
      xyzc_[0]->reset();
      xyzc_[1]->reset();
      if (rk_ != FloatKey()) {
        rc_[0]->reset();
        rc_[1]->reset();
      }
    }
  }
}


void CloseBipartitePairsScoreState::show(std::ostream &out) const
{
  out << "CloseBipartitePairsScoreState" << std::endl;
}

IMPCORE_END_NAMESPACE
