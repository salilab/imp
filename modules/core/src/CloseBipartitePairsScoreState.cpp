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
                                FloatKey rk):
  ScoreState("CloseBipartitePairsScoreState %1%")
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
                                FloatKey rk):
  ScoreState("CloseBipartitePairsScoreState %1%")
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
    xyzc_[0] =new MaximumChangeXYZRScoreState(in_[0],rk_);
    xyzc_[1] =new MaximumChangeXYZRScoreState(in_[1],rk_);
    IMP_LOG(TERSE, "adding pairs" << std::endl);
    out_->clear_particle_pairs();
    ParticlePairsTemp cp= f_->get_close_pairs(in_[0], in_[1]);
    cp.erase(std::remove_if(cp.begin(),
                            cp.end(),
                            Found(close_pair_filters_begin(),
                                  close_pair_filters_end())),
             cp.end());
    out_->set_particle_pairs(cp);
    IMP_LOG(TERSE, "done"<< std::endl);
    return;
  } else {
    xyzc_[0]->before_evaluate(ScoreState::get_before_evaluate_iteration());
    xyzc_[1]->before_evaluate(ScoreState::get_before_evaluate_iteration());
    Float delta= xyzc_[0]->get_maximum_change()
               + xyzc_[1]->get_maximum_change();

    if (delta > slack_) {
      out_->clear_particle_pairs();
      ParticlePairsTemp cp= f_->get_close_pairs(in_[0], in_[1]);
      cp.erase(std::remove_if(cp.begin(),
                              cp.end(),
                              Found(close_pair_filters_begin(),
                                    close_pair_filters_end())),
               cp.end());
      out_->set_particle_pairs(cp);
      xyzc_[0]->reset();
      xyzc_[1]->reset();
    }
  }
}

void CloseBipartitePairsScoreState
::do_after_evaluate(DerivativeAccumulator *){
}


ParticlesList CloseBipartitePairsScoreState::get_interacting_particles() const {
  return ParticlesList();
}

ParticlesTemp CloseBipartitePairsScoreState::get_used_particles() const {
  ParticlesTemp ret0(f_->get_used_particles(in_[0]));
  ParticlesTemp ret1(f_->get_used_particles(in_[1]));
  ret0.insert(ret0.end(), ret1.begin(), ret1.end());
  ParticlePairsTemp all_pairs;
  for (unsigned int i=0; i< ret0.size(); ++i) {
    for (unsigned int j=0; j< ret1.size(); ++j) {
      all_pairs.push_back(ParticlePair(ret0[i], ret1[j]));
    }
  }
  for (ClosePairFilterConstIterator it= close_pair_filters_begin();
       it != close_pair_filters_end(); ++it) {
    ParticlesTemp cur= (*it)->get_used_particles(all_pairs);
    ret0.insert(ret0.end(), cur.begin(), cur.end());
  }
  ret0.insert(ret0.end(), ret1.begin(), ret1.end());
  return ret0;
}

void CloseBipartitePairsScoreState::show(std::ostream &out) const
{
  out << "CloseBipartitePairsScoreState" << std::endl;
}



IMP_LIST_IMPL(CloseBipartitePairsScoreState,
              ClosePairFilter,
              close_pair_filter,
              PairFilter*,
              PairFilters,,,)

IMPCORE_END_NAMESPACE
