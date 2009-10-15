/**
 *  \file ClosePairsScoreState.cpp
 *  \brief Keep track of the maximumimum change of a set of attributes.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */


#include <IMP/core/ClosePairsScoreState.h>
#include <IMP/core/GridClosePairsFinder.h>
#include <IMP/core/BoxSweepClosePairsFinder.h>

#include <algorithm>
#include <sstream>

IMPCORE_BEGIN_NAMESPACE

namespace {
  class Found {
    typedef ClosePairsScoreState
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


ClosePairsScoreState::ClosePairsScoreState(SingletonContainer *pc,
                                           ListPairContainer* out,
                                           FloatKey rk):
  ScoreState("ClosePairsScoreState %1%"),
  in_(pc),
  out_(out)
{
  rk_=rk;
  initialize();
}

ClosePairsScoreState::ClosePairsScoreState(SingletonContainer *pc,
                                           FloatKey rk):
  ScoreState("ClosePairsScoreState %1%"),
  in_(pc)
{
  out_=new ListPairContainer();
  rk_=rk;
  initialize();
}


void ClosePairsScoreState::initialize() {
  distance_=0;
  slack_=1;
#ifdef IMP_USE_CGAL
  set_close_pairs_finder(new BoxSweepClosePairsFinder());
#else
  set_close_pairs_finder(new GridClosePairsFinder());
#endif
}

void ClosePairsScoreState::set_distance(Float d) {
  distance_=d;
  f_->set_distance(distance_+slack_);
}

void ClosePairsScoreState::set_slack(Float d) {
  slack_=d;
  f_->set_distance(distance_+slack_);
}

void ClosePairsScoreState::set_singleton_container(SingletonContainer *pc) {
  // needs to be first for the case of assigning the pc that is already there
  in_=pc;
  xyzc_->set_singleton_container(in_);
}

void ClosePairsScoreState::set_close_pairs_finder(ClosePairsFinder *f) {
  f_=f;
  f_->set_distance(distance_+slack_);
  f_->set_radius_key(rk_);
}

void ClosePairsScoreState::set_radius_key(FloatKey k) {
  rk_=k;
  xyzc_=NULL;
  f_->set_radius_key(rk_);
}


namespace {
  struct IsInactive {
    bool operator()(const ParticlePair &p) const {
      return !p[0]->get_is_active() || !p[1]->get_is_active();
    }
  };
}

void ClosePairsScoreState::do_before_evaluate()
{
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(in_);
  IMP_CHECK_OBJECT(out_);
  IMP_CHECK_OBJECT(f_);
  if (!xyzc_) {
    //std::cout << "Virgin ss" << std::endl;
    xyzc_ =new MaximumChangeXYZRScoreState(in_, rk_);
    //std::cout << "adding pairs" << std::endl;
    unsigned int sz= out_->get_number_of_particle_pairs();
    out_->clear_particle_pairs();
    out_->reserve_particle_pairs(sz);
    ParticlePairsTemp cp= f_->get_close_pairs(in_);
    cp.erase(std::remove_if(cp.begin(),
                            cp.end(),
                            Found(close_pair_filters_begin(),
                                  close_pair_filters_end())),
             cp.end());
    out_->set_particle_pairs(cp);
    return;
  } else {
    xyzc_->before_evaluate(ScoreState::get_before_evaluate_iteration());
    Float delta= xyzc_->get_maximum_change();
    if (delta*2 > slack_) {
      ParticlePairsTemp cp= f_->get_close_pairs(in_);
      cp.erase(std::remove_if(cp.begin(),
                              cp.end(),
                              Found(close_pair_filters_begin(),
                                    close_pair_filters_end())),
               cp.end());
      out_->set_particle_pairs(cp);
      xyzc_->reset();
    }
  }
}
void ClosePairsScoreState::do_after_evaluate(DerivativeAccumulator*){
}

ParticlesList ClosePairsScoreState::get_interacting_particles() const {
  return ParticlesList();
}

ParticlesTemp ClosePairsScoreState::get_read_particles() const {
  ParticlesTemp ret(f_->get_used_particles(in_));
  ParticlePairsTemp all_pairs;
  for (unsigned int i=0; i< ret.size(); ++i) {
    for (unsigned int j=0; j< i; ++j) {
      all_pairs.push_back(ParticlePair(ret[i], ret[j]));
    }
  }
  for (ClosePairFilterConstIterator it= close_pair_filters_begin();
       it != close_pair_filters_end(); ++it) {
    ParticlesTemp cur= (*it)->get_used_particles(all_pairs);
    ret.insert(ret.end(), cur.begin(), cur.end());
  }
  return ret;
}

ParticlesTemp ClosePairsScoreState::get_write_particles() const {
  return ParticlesTemp();
}


void ClosePairsScoreState::show(std::ostream &out) const
{
  out << "ClosePairsScoreState" << std::endl;
}




IMP_LIST_IMPL(ClosePairsScoreState,
              ClosePairFilter,
              close_pair_filter,
              PairFilter*,
              PairFilters,,,)
IMPCORE_END_NAMESPACE
