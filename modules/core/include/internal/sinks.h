/**
 *  \file rigid_pair_score.h
 *  \brief utilities for rigid pair scores.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_SINKS_H
#define IMPCORE_INTERNAL_SINKS_H

#include <IMP/core/core_config.h>
#include "IMP/Particle.h"
#include "IMP/PairPredicate.h"
#include "rigid_body_tree.h"

IMPCORE_BEGIN_INTERNAL_NAMESPACE

inline bool get_filters_contains(Model *m,
                                 const PairPredicates &filters,
                                 ParticleIndexPair pip) {
  for (unsigned int i=0; i< filters.size(); ++i) {
    if (filters[i]->get_value_index(m, pip)) return true;
  }
  return false;
}

struct ParticleSink {
  Model *m_;
  ParticlesTemp &out_;
  ParticleSink(Model *m, ParticlesTemp &out): m_(m), out_(out){}
  typedef ParticleIndex argument_type;
  bool operator()( ParticleIndex c) {
    out_.push_back(m_->get_particle(c));
    return true;
  }
  void check_contains(ParticleIndex c) const {
    if (std::find(out_.begin(), out_.end(), m_->get_particle(c))
        == out_.end()) {
      IMP_INTERNAL_CHECK(false, "Particle " << c
                         << " not found in list. Coordinates are "
                         << XYZR(m_, c));
    }
  }
};


struct ParticlePairSink {
  Model *m_;
  const PairPredicates & filters_;
  ParticlePairsTemp &out_;
  ParticlePairSink(Model *m, const PairPredicates &filters,
                   ParticlePairsTemp &out): m_(m), filters_(filters),
                                            out_(out){}
  bool add(ParticleIndex a, ParticleIndex b) {
    if (get_filters_contains(m_, filters_,
                             ParticleIndexPair(a,b))) return false;
    out_.push_back(ParticlePair(m_->get_particle(a),
                                m_->get_particle(b)));
    return true;
  }
  bool operator()(ParticleIndex a, ParticleIndex b) {
    add(a,b);
    return true;
  }
  void check_contains(ParticleIndex a, ParticleIndex b) const {
    if (get_filters_contains(m_, filters_, ParticleIndexPair(a,b))) return;
    ParticlePair pp(m_->get_particle(a),
                    m_->get_particle(b));
    ParticlePair opp(m_->get_particle(b),
                    m_->get_particle(a));
    if (std::find(out_.begin(), out_.end(), pp) == out_.end()
        && std::find(out_.begin(), out_.end(), opp)
        == out_.end()) {
      IMP_INTERNAL_CHECK(false, "Particle pair " << pp[0]->get_name()
                         << ", " << pp[1]->get_name()
                         << " not found in list. Coordinates are "
                         << XYZR(pp[0]) << " and " << XYZR(pp[1])
                         << " and distance is " << get_distance(XYZR(pp[0]),
                                                                XYZR(pp[1])));
    }
  }
};


struct ParticleIndexPairSink {
  Model *m_;
  const PairPredicates & filters_;
  ParticleIndexPairs &out_;
  ParticleIndexPairSink(Model *m, const PairPredicates &filters,
                        ParticleIndexPairs &out):
    m_(m), filters_(filters), out_(out){}
  bool add(ParticleIndex a, ParticleIndex b) {
    if (get_filters_contains(m_, filters_,
                             ParticleIndexPair(a,b))) return false;
    out_.push_back(ParticleIndexPair(a,b));
    return true;
  }
  bool operator()(ParticleIndex a, ParticleIndex b) {
    add(a,b);
    return true;
  }
  void check_contains(ParticleIndex a, ParticleIndex b) const {
    if (get_filters_contains(m_, filters_, ParticleIndexPair(a,b))) return;
    ParticleIndexPair pp(a,b);
    ParticleIndexPair opp(b,a);
    if (std::find(out_.begin(), out_.end(), pp) == out_.end()
        && std::find(out_.begin(), out_.end(), opp)
        == out_.end()) {
      IMP_INTERNAL_CHECK(false, "Particle pair " << pp
                         << " not found in list.");
    }
  }
};


template <class PS>
struct ParticlePairSinkWithMax: public ParticlePairSink {
  double &score_;
  double max_;
  PS *ssps_;
  DerivativeAccumulator *da_;
  ParticlePairSinkWithMax(Model *m,
                          const PairPredicates &filters,
                          ParticlePairsTemp &out,
                          PS *ssps,
                          DerivativeAccumulator *da,
                          double &score,
                          double max):
    ParticlePairSink(m, filters, out),
    score_(score),
    max_(max),
    ssps_(ssps), da_(da) {}
  bool operator()(ParticleIndex a, ParticleIndex b) {
    if (!ParticlePairSink::add(a,b)) return true;
    double cur= ssps_->PS::evaluate(ParticlePair(m_->get_particle(a),
                                                 m_->get_particle(b)), da_);
    max_-=cur;
    score_+=cur;
    if (max_ < 0) {
      return false;
    }
    return true;
  }
};



template <class PS>
struct ParticleIndexPairSinkWithMax: public ParticleIndexPairSink {
  double &score_;
  double max_;
  PS *ssps_;
  DerivativeAccumulator *da_;
  ParticleIndexPairSinkWithMax(Model *m,
                               const PairPredicates &filters,
                               ParticleIndexPairs &out,
                          PS *ssps,
                          DerivativeAccumulator *da,
                          double &score,
                          double max):
    ParticleIndexPairSink(m, filters, out),
    score_(score),
    max_(max),
    ssps_(ssps), da_(da) {}
  bool operator()(ParticleIndex a, ParticleIndex b) {
    if (!ParticleIndexPairSink::add(a,b)) return true;
    double cur= ssps_->PS::evaluate_index(ParticleIndexPairSink::m_,
                                          ParticleIndexPair(a,b), da_);
    max_-=cur;
    score_+=cur;
    if (max_ < 0) {
      return false;
    }
    return true;
  }
};

struct HalfParticlePairSink: public ParticlePairSink {
  ParticleIndex p_;
  HalfParticlePairSink(Model *m, const PairPredicates &filters,
                       ParticlePairsTemp &out,
                       ParticleIndex p):
    ParticlePairSink(m, filters, out),
    p_(p) {}
  bool operator()(ParticleIndex c) {
    return ParticlePairSink::operator()(p_, c);
  }
  void check_contains(ParticleIndex c) const {
    ParticlePairSink::check_contains(p_, c);
  }
};

struct HalfParticleIndexPairSink: public ParticleIndexPairSink {
  ParticleIndex p_;
  HalfParticleIndexPairSink(Model *m, const PairPredicates &filters,
                            ParticleIndexPairs &out,
                            ParticleIndex p):
    ParticleIndexPairSink(m, filters, out),
    p_(p) {}
  bool operator()(ParticleIndex c) {
    return ParticleIndexPairSink::operator()(p_, c);
  }
  void check_contains(ParticleIndex c) const {
    ParticleIndexPairSink::check_contains(p_, c);
  }
};

struct SwappedHalfParticleIndexPairSink: public ParticleIndexPairSink {
  ParticleIndex p_;
  SwappedHalfParticleIndexPairSink(Model *m, const PairPredicates &filters,
                            ParticleIndexPairs &out,
                            ParticleIndex p):
    ParticleIndexPairSink(m, filters, out),
    p_(p) {}
  bool operator()(ParticleIndex c) {
    return ParticleIndexPairSink::operator()(c, p_);
  }
  void check_contains(ParticleIndex c) const {
    ParticleIndexPairSink::check_contains(c, p_);
  }
};

template <class PS>
struct HalfParticlePairSinkWithMax: public ParticlePairSinkWithMax<PS> {
  ParticleIndex p_;
  HalfParticlePairSinkWithMax(Model *m, const PairPredicates &filters,
                              ParticlePairsTemp &out,
                              PS *ssps,
                              DerivativeAccumulator *da,
                              double &score,
                              double max, ParticleIndex p):
    ParticlePairSinkWithMax<PS>(m, filters, out, ssps, da, score, max),
    p_(p) {}
  bool operator()(ParticleIndex c) {
    return ParticlePairSinkWithMax<PS>::operator()(p_, c);
  }
  void check_contains(ParticleIndex c) const {
    ParticlePairSinkWithMax<PS>::check_contains(p_, c);
  }
};

template <class PS>
struct HalfParticleIndexPairSinkWithMax:
  public ParticleIndexPairSinkWithMax<PS> {
  ParticleIndex p_;
  HalfParticleIndexPairSinkWithMax(Model *m,
                                   const PairPredicates &filters,
                              ParticleIndexPairs &out,
                              PS *ssps,
                              DerivativeAccumulator *da,
                              double &score,
                              double max, ParticleIndex p):
    ParticleIndexPairSinkWithMax<PS>(m, filters, out, ssps, da, score, max),
    p_(p) {}
  bool operator()(ParticleIndex c) {
    return ParticleIndexPairSinkWithMax<PS>::operator()(p_, c);
  }
  void check_contains(ParticleIndex c) const {
    ParticleIndexPairSinkWithMax<PS>::check_contains(p_, c);
  }
};


struct RigidBodyRigidBodyParticleIndexPairSink:
  public ParticleIndexPairSink {
  ObjectKey key_;
  double dist_;
  const IMP::base::map<ParticleIndex,
                                ParticleIndexes> &map_;
  RigidBodyRigidBodyParticleIndexPairSink(Model *m,
                                          const PairPredicates &filters,
                                     ParticleIndexPairs &out,
                                     ObjectKey key,
                                     double dist,
                      const IMP::base::map<ParticleIndex,
                                ParticleIndexes>
                                     &map):
    ParticleIndexPairSink(m, filters, out),
    key_(key), dist_(dist), map_(map){}
  RigidBodyHierarchy *get_hierarchy(ParticleIndex p) const {
    RigidBody rb(m_, p);
    return get_rigid_body_hierarchy(rb, map_.find(p)->second, key_);
  }
  bool operator()(ParticleIndex a, ParticleIndex b) {
    IMP_LOG_VERBOSE( "Processing interesction between "
            << a << " and "
            << b << std::endl);
    fill_close_pairs(m_, get_hierarchy(a),
                     get_hierarchy(b),
                     dist_, static_cast<ParticleIndexPairSink >(*this));
    return true;
  }
  void check_contains(ParticleIndex, ParticleIndex) const {
    // can't look for root pair, too lazy to check for actual pairs
  }
};


struct RigidBodyParticleParticleIndexPairSink:
  public ParticleIndexPairSink {
  ObjectKey key_;
  double dist_;
  const IMP::base::map<ParticleIndex,
                                ParticleIndexes> &map_;
  RigidBodyParticleParticleIndexPairSink(Model *m,
                                         const PairPredicates &filters,
                                    ParticleIndexPairs &out,
                                    ObjectKey key, double dist,
               const IMP::base::map<ParticleIndex,
                                    ParticleIndexes> &map):
    ParticleIndexPairSink(m, filters, out),
    key_(key), dist_(dist), map_(map){}
  RigidBodyHierarchy *get_hierarchy(ParticleIndex p) const {
    RigidBody rb(m_, p);
    return get_rigid_body_hierarchy(rb, map_.find(p)->second, key_);
  }
  bool operator()(ParticleIndex a, ParticleIndex b) {
    IMP_LOG_VERBOSE( "Processing rb-p interesction between "
            << a << " and "
            << b << std::endl);
    SwappedHalfParticleIndexPairSink hps(m_, filters_, out_, b);
    fill_close_particles(m_, get_hierarchy(a),
                         b,
                         dist_, hps);
    return true;
  }
  void check_contains(ParticleIndex, ParticleIndex) const {
    // can't look for root pair, too lazy to check for actual pairs
  }
};



struct ParticleRigidBodyParticleIndexPairSink:
  public ParticleIndexPairSink {
  ObjectKey key_;
  double dist_;
  const IMP::base::map<ParticleIndex,
                                ParticleIndexes> &map_;
  ParticleRigidBodyParticleIndexPairSink(Model *m,
                                         const PairPredicates &filters,
                                    ParticleIndexPairs &out,
                                    ObjectKey key, double dist,
               const IMP::base::map<ParticleIndex,
                                    ParticleIndexes> &map):
    ParticleIndexPairSink(m, filters, out),
    key_(key), dist_(dist), map_(map){}
  RigidBodyHierarchy *get_hierarchy(ParticleIndex p) const {
    RigidBody rb(m_, p);
    return get_rigid_body_hierarchy(rb, map_.find(p)->second, key_);
  }
  bool operator()(ParticleIndex a, ParticleIndex b) {
    IMP_LOG_VERBOSE( "Processing p-rb interesction between "
            << a << " and "
            << b << std::endl);
    HalfParticleIndexPairSink hps(m_, filters_, out_, a);
    fill_close_particles(m_, get_hierarchy(b),
                         a,
                         dist_, hps);
    return true;
  }
  void check_contains(ParticleIndex, ParticleIndex) const {
    // can't look for root pair, too lazy to check for actual pairs
  }
};


template <class PS>
struct RigidBodyParticleIndexPairSinkWithMax:
  public ParticleIndexPairSinkWithMax<PS> {
  ObjectKey key_;
  double dist_;
  const IMP::base::map<ParticleIndex,
                                ParticleIndexes> &map_;
  RigidBodyParticleIndexPairSinkWithMax(Model *m,
                                        const PairPredicates &filters,
                                   ParticleIndexPairs &out,
                                   PS *ssps,
                                   DerivativeAccumulator *da,
                                   double &score,
                                   double max,
                                   ObjectKey key,
                                   double dist,
  const IMP::base::map<ParticleIndex,
                                ParticleIndexes> &map):
    ParticleIndexPairSinkWithMax<PS>(m, filters, out, ssps, da, score, max),
    key_(key), dist_(dist), map_(map){}
  RigidBodyHierarchy *get_hierarchy(ParticleIndex p) const {
    RigidBody rb(ParticleIndexPairSinkWithMax<PS>::m_, p);
    return get_rigid_body_hierarchy(rb, map_.find(p)->second, key_);
  }
  void check_contains(ParticleIndex, ParticleIndex) const {
    // can't look for root pair, too lazy to check for actual pairs
  }
};


template <class PS>
struct RigidBodyRigidBodyParticleIndexPairSinkWithMax:
  public RigidBodyParticleIndexPairSinkWithMax<PS> {
  typedef RigidBodyParticleIndexPairSinkWithMax<PS> P;
  RigidBodyRigidBodyParticleIndexPairSinkWithMax
  (Model *m,
   const PairPredicates &filters,
   ParticleIndexPairs &out,
   PS *ssps,
   DerivativeAccumulator *da,
   double &score,
   double max,
   ObjectKey key, double dist,
   const IMP::base::map<ParticleIndex,
                                ParticleIndexes> &map):
    P(m, filters, out, ssps, da,
      score, max,
      key, dist, map)
  {}
  bool operator()(ParticleIndex a, ParticleIndex b) {
    fill_close_pairs(P::m_,
            RigidBodyParticleIndexPairSinkWithMax<PS>::get_hierarchy(a),
            RigidBodyParticleIndexPairSinkWithMax<PS>::get_hierarchy(b),
                     P::dist_,
                     static_cast<ParticleIndexPairSinkWithMax<PS> >(*this));
    return P::max_ > 0;
  }
  void check_contains(ParticleIndex, ParticleIndex) const {
    // can't look for root pair, too lazy to check for actual pairs
  }
};

template <class PS>
struct RigidBodyParticleParticleIndexPairSinkWithMax:
  public RigidBodyParticleIndexPairSinkWithMax<PS> {
  typedef RigidBodyParticleIndexPairSinkWithMax<PS> P;
  RigidBodyParticleParticleIndexPairSinkWithMax
  (Model *m,
   const PairPredicates &filters,
   ParticleIndexPairs &out,
   PS *ssps,
   DerivativeAccumulator *da,
   double &score,
   double max,
   ObjectKey key, double dist,
   const IMP::base::map<ParticleIndex,
   ParticleIndexes>
   &map): P(m, filters, out, ssps, da,
            score, max, key, dist, map)
  {}
  bool operator()(ParticleIndex a, ParticleIndex b) {
    fill_close_particles(P::m_,
                         P::get_hierarchy(a),
                         b,
                         P::dist_,
                         HalfParticleIndexPairSinkWithMax<PS>(P::m_,
                                                         P::filters_,
                                                         P::out_,
                                                         P::ssps_,
                                                         P::da_,
                                                         P::score_,
                                                         P::max_, b));
    return P::max_ > 0;
  }
  void check_contains(ParticleIndex, ParticleIndex) const {
    // can't look for root pair, too lazy to check for actual pairs
  }
};

IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_SINKS_H */
