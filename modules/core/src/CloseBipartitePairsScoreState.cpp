/**
 *  \file CloseBipartitePairsScoreState.cpp
 *  \brief Keep track of the maximumimum change of a set of attributes.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */


#include <IMP/core/CloseBipartitePairsScoreState.h>
#include <IMP/core/QuadraticClosePairsFinder.h>

#include <algorithm>
#include <sstream>

IMPCORE_BEGIN_NAMESPACE

namespace {
  class Found {
    typedef CloseBipartitePairsScoreState
    ::ClosePairFilterIterator It;
    It b_,e_;
  public:
    Found(It b,
          It e):
      b_(b), e_(e){}
    bool operator()(ParticlePair vt) const {
      if (vt.first==vt.second) return true;
      for (It c=b_; c != e_; ++c) {
        if ((*c)->get_contains_particle_pair(vt)) return true;
      }
      return false;
    }
  };
}

CloseBipartitePairsScoreState
::CloseBipartitePairsScoreState(SingletonContainer *pc0,
                                SingletonContainer *pc1,
                                ListPairContainer* out,
                                FloatKey rk)
{
  in_[0]=pc0;
  in_[1]=pc1;
  out_=out;
  rk_=rk;
  initialize();
}

CloseBipartitePairsScoreState
::CloseBipartitePairsScoreState(SingletonContainer *pc0,
                                SingletonContainer *pc1,
                                           FloatKey rk)
{
  in_[0]=pc0;
  in_[1]=pc1;
  out_=new ListPairContainer();
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
  f_->set_distance(slack_+distance_);
}

void CloseBipartitePairsScoreState::set_slack(Float d) {
  slack_=d;
  f_->set_distance(distance_+slack_);
}

void CloseBipartitePairsScoreState::clear() {
  xyzc_[0]=NULL;
  xyzc_[1]=NULL;
  rc_[0]=NULL;
  rc_[1]=NULL;
}

void CloseBipartitePairsScoreState
::set_first_singleton_container(SingletonContainer *pc) {
  // needs to be first for the case of assigning the pc that is already there
  in_[0]=pc;
  clear();
}

void CloseBipartitePairsScoreState
::set_second_singleton_container(SingletonContainer *pc) {
  in_[1]=pc;
  clear();
}

void CloseBipartitePairsScoreState
::set_close_pairs_finder(ClosePairsFinder *f) {
  f_=f;
  f_->set_radius_key(rk_);
  f_->set_distance(distance_+slack_);
}

void CloseBipartitePairsScoreState::set_radius_key(FloatKey k) {
  rk_=k;
  f_->set_radius_key(rk_);
  clear();
}


namespace {
  struct IsInactive {
    bool operator()(const ParticlePair &p) const {
      return !p[0]->get_is_active() || !p[1]->get_is_active();
    }
  };
}

void CloseBipartitePairsScoreState::do_before_evaluate()
{
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(in_[0]);
  IMP_CHECK_OBJECT(in_[1]);
  IMP_CHECK_OBJECT(out_);
  IMP_CHECK_OBJECT(f_);
  if (!xyzc_[0]) {
    IMP_LOG(TERSE, "Virgin ss" << std::endl);
    xyzc_[0] =new MaximumChangeScoreState(in_[0], XYZ::get_xyz_keys());
    xyzc_[1] =new MaximumChangeScoreState(in_[1], XYZ::get_xyz_keys());
    if (rk_ != FloatKey()) {
      rc_[0]= new MaximumChangeScoreState(in_[0], FloatKeys(1, rk_));
      rc_[1]= new MaximumChangeScoreState(in_[1], FloatKeys(1, rk_));
    }
    IMP_LOG(TERSE, "adding pairs" << std::endl);
    out_->clear_particle_pairs();
    f_->add_close_pairs(in_[0], in_[1],out_);
    out_->remove_particle_pairs_if(Found(close_pair_filters_begin(),
                                         close_pair_filters_end()));
    IMP_LOG(TERSE, "done"<< std::endl);
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
      f_->add_close_pairs(in_[0], in_[1],out_);
      out_->remove_particle_pairs_if(Found(close_pair_filters_begin(),
                                           close_pair_filters_end()));
      xyzc_[0]->reset();
      xyzc_[1]->reset();
      if (rk_ != FloatKey()) {
        rc_[0]->reset();
        rc_[1]->reset();
      }
    }
  }
}

void CloseBipartitePairsScoreState
::do_after_evaluate(DerivativeAccumulator *){
}


void CloseBipartitePairsScoreState::show(std::ostream &out) const
{
  out << "CloseBipartitePairsScoreState" << std::endl;
}



IMP_LIST_IMPL(CloseBipartitePairsScoreState,
              ClosePairFilter,
              close_pair_filter,
              PairContainer*,
              PairContainers,,,)

IMPCORE_END_NAMESPACE
