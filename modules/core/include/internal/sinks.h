/**
 *  \file rigid_pair_score.h
 *  \brief utilities for rigid pair scores.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_SINKS_H
#define IMPCORE_INTERNAL_SINKS_H

#include "../core_config.h"
#include "IMP/Particle.h"
#include "rigid_body_tree.h"

IMPCORE_BEGIN_INTERNAL_NAMESPACE

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
  ParticlePairsTemp &out_;
  ParticlePairSink(Model *m, ParticlePairsTemp &out): m_(m), out_(out){}
  bool operator()(ParticleIndex a, ParticleIndex b) {
    out_.push_back(ParticlePair(m_->get_particle(a),
                                m_->get_particle(b)));
    return true;
  }
  void check_contains(ParticleIndex a, ParticleIndex b) const {
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


template <class PS>
struct ParticlePairSinkWithMax: public ParticlePairSink {
  double &score_;
  double max_;
  PS *ssps_;
  DerivativeAccumulator *da_;
  ParticlePairSinkWithMax(Model *m, ParticlePairsTemp &out,
                          PS *ssps,
                          DerivativeAccumulator *da,
                          double &score,
                          double max):
    ParticlePairSink(m, out),
    score_(score),
    max_(max),
    ssps_(ssps), da_(da) {}
  bool operator()(ParticleIndex a, ParticleIndex b) {
    ParticlePairSink::operator()(a,b);
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

struct HalfParticlePairSink: public ParticlePairSink {
  ParticleIndex p_;
  HalfParticlePairSink(Model *m, ParticlePairsTemp &out,
                       ParticleIndex p):
    ParticlePairSink(m, out),
    p_(p) {}
  bool operator()(ParticleIndex c) {
    return ParticlePairSink::operator()(p_, c);
  }
  void check_contains(ParticleIndex c) const {
    ParticlePairSink::check_contains(p_, c);
  }
};

template <class PS>
struct HalfParticlePairSinkWithMax: public ParticlePairSinkWithMax<PS> {
  ParticleIndex p_;
  HalfParticlePairSinkWithMax(Model *m,
                              ParticlePairsTemp &out,
                              PS *ssps,
                              DerivativeAccumulator *da,
                              double &score,
                              double max, ParticleIndex p):
    ParticlePairSinkWithMax<PS>(m, out, ssps, da, score, max),
    p_(p) {}
  bool operator()(ParticleIndex c) {
    return ParticlePairSinkWithMax<PS>::operator()(p_, c);
  }
  void check_contains(ParticleIndex c) const {
    ParticlePairSinkWithMax<PS>::check_contains(p_, c);
  }
};


struct RigidBodyRigidBodyParticlePairSink:
  public ParticlePairSink {
  ObjectKey key_;
  double dist_;
  const IMP::compatibility::map<RigidBody, Particles> &map_;
  RigidBodyRigidBodyParticlePairSink(Model *m,
                                     ParticlePairsTemp &out,
                                     ObjectKey key,
                                     double dist,
                      const IMP::compatibility::map<RigidBody, Particles>
                                     &map):
    ParticlePairSink(m, out),
    key_(key), dist_(dist), map_(map){}
  RigidBodyHierarchy *get_hierarchy(ParticleIndex p) const {
    RigidBody rb(m_, p);
    return get_rigid_body_hierarchy(rb, map_.find(rb)->second, key_);
  }
  bool operator()(ParticleIndex a, ParticleIndex b) {
    IMP_LOG(VERBOSE, "Processing interesction between "
            << a << " and "
            << b << std::endl);
    fill_close_pairs(m_, get_hierarchy(a),
                     get_hierarchy(b),
                     dist_, static_cast<ParticlePairSink >(*this));
    return true;
  }
  void check_contains(ParticleIndex, ParticleIndex) const {
    // can't look for root pair, too lazy to check for actual pairs
  }
};


struct RigidBodyParticleParticlePairSink:
  public ParticlePairSink {
  ObjectKey key_;
  double dist_;
  const IMP::compatibility::map<RigidBody, Particles> &map_;
  RigidBodyParticleParticlePairSink(Model *m,
                                    ParticlePairsTemp &out,
                                    ObjectKey key, double dist,
               const IMP::compatibility::map<RigidBody, Particles>
                                     &map):
    ParticlePairSink(m, out),
    key_(key), dist_(dist), map_(map){}
  RigidBodyHierarchy *get_hierarchy(ParticleIndex p) const {
    RigidBody rb(m_, p);
    return get_rigid_body_hierarchy(rb, map_.find(rb)->second, key_);
  }
  bool operator()(ParticleIndex a, ParticleIndex b) {
    IMP_LOG(VERBOSE, "Processing rb-p interesction between "
            << a << " and "
            << b << std::endl);
    HalfParticlePairSink hps(m_, out_, b);
    fill_close_particles(m_, get_hierarchy(a),
                         b,
                         dist_, hps);
    return true;
  }
  void check_contains(ParticleIndex, ParticleIndex) const {
    // can't look for root pair, too lazy to check for actual pairs
  }
};

template <class PS>
struct RigidBodyParticlePairSinkWithMax:
  public ParticlePairSinkWithMax<PS> {
  ObjectKey key_;
  double dist_;
  const IMP::compatibility::map<RigidBody, Particles> &map_;
  RigidBodyParticlePairSinkWithMax(Model *m,
                                   ParticlePairsTemp &out,
                                   PS *ssps,
                                   DerivativeAccumulator *da,
                                   double &score,
                                   double max,
                                   ObjectKey key,
                                   double dist,
               const IMP::compatibility::map<RigidBody, Particles>
                                     &map):
    ParticlePairSinkWithMax<PS>(m, out, ssps, da, score, max),
    key_(key), dist_(dist), map_(map){}
  RigidBodyHierarchy *get_hierarchy(ParticleIndex p) const {
    RigidBody rb(ParticlePairSinkWithMax<PS>::m_, p);
    return get_rigid_body_hierarchy(rb, map_.find(rb)->second, key_);
  }
  void check_contains(ParticleIndex, ParticleIndex) const {
    // can't look for root pair, too lazy to check for actual pairs
  }
};

template <class PS>
struct RigidBodyRigidBodyParticlePairSinkWithMax:
  public RigidBodyParticlePairSinkWithMax<PS> {
  typedef RigidBodyParticlePairSinkWithMax<PS> P;
  RigidBodyRigidBodyParticlePairSinkWithMax
  (Model *m,
   ParticlePairsTemp &out,
   PS *ssps,
   DerivativeAccumulator *da,
   double &score,
   double max,
   ObjectKey key, double dist,
   const IMP::compatibility::map<RigidBody, Particles>
   &map): P(m, out, ssps, da,
            score, max,
            key, dist, map)
  {}
  bool operator()(ParticleIndex a, ParticleIndex b) {
    fill_close_pairs(P::m_,
                     RigidBodyParticlePairSinkWithMax<PS>::get_hierarchy(a),
                     RigidBodyParticlePairSinkWithMax<PS>::get_hierarchy(b),
                     P::dist_,
                     static_cast<ParticlePairSinkWithMax<PS> >(*this));
    return P::max_ > 0;
  }
  void check_contains(ParticleIndex, ParticleIndex) const {
    // can't look for root pair, too lazy to check for actual pairs
  }
};

template <class PS>
struct RigidBodyParticleParticlePairSinkWithMax:
  public RigidBodyParticlePairSinkWithMax<PS> {
  typedef RigidBodyParticlePairSinkWithMax<PS> P;
  RigidBodyParticleParticlePairSinkWithMax
  (Model *m,
   ParticlePairsTemp &out,
   PS *ssps,
   DerivativeAccumulator *da,
   double &score,
   double max,
   ObjectKey key, double dist,
   const IMP::compatibility::map<RigidBody, Particles>
   &map): P(m, out, ssps, da,
            score, max, key, dist, map)
  {}
  bool operator()(ParticleIndex a, ParticleIndex b) {
    fill_close_particles(P::m_,
                         P::get_hierarchy(a),
                         b,
                         P::dist_,
                         HalfParticlePairSinkWithMax<PS>(P::m_,
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
