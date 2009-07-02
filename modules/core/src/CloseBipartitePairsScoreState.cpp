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
  class DegeneratePairContainer: public PairContainer {
  public:
    DegeneratePairContainer(){}
    unsigned int get_number_of_particle_pairs() const {return 0;}
    ParticlePair get_particle_pair(unsigned int) const {
      IMP_failure("The container contains no pairs", ErrorException);
    }
    bool get_contains_particle_pair(ParticlePair pp) const {
      return pp.first==pp.second;
    }
    void show(std::ostream &out) const {
    }
    VersionInfo get_version_info() const {
      return internal::version_info;
    }
  };
}

CloseBipartitePairsScoreState
::CloseBipartitePairsScoreState(SingletonContainer *pc0,
                                SingletonContainer *pc1,
                                FilteredListPairContainer* out,
                                FloatKey rk)
{
  in_[0]=pc0;
  in_[1]=pc1;
  out_=out;
  rk_=rk;
  initialize();
  out_->add_pair_filter(new DegeneratePairContainer());
}

CloseBipartitePairsScoreState
::CloseBipartitePairsScoreState(SingletonContainer *pc0,
                                SingletonContainer *pc1,
                                           FloatKey rk)
{
  in_[0]=pc0;
  in_[1]=pc1;
  out_=new FilteredListPairContainer();
  out_->add_pair_filter(new DegeneratePairContainer());
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
      xyzc_[0]->reset();
      xyzc_[1]->reset();
      if (rk_ != FloatKey()) {
        rc_[0]->reset();
        rc_[1]->reset();
      }
    } else {
      out_->remove_particle_pairs_if(IsInactive());
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

IMPCORE_END_NAMESPACE
