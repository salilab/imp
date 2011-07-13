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
  ParticlesTemp &out_;
  ParticleSink(ParticlesTemp &out): out_(out){}
  typedef Particle* argument_type;
  bool operator()( Particle* c) {
    out_.push_back(c);
    return true;
  }
  void check_contains(Particle *c) const {
    if (std::find(out_.begin(), out_.end(), c) == out_.end()) {
      IMP_INTERNAL_CHECK(false, "Particle " << c->get_name()
                         << " not found in list. Coordinates are "
                         << XYZR(c));
    }
  }
};

struct ParticlePairSink {
  ParticlePairsTemp &out_;
  ParticlePairSink(ParticlePairsTemp &out): out_(out){}
  typedef ParticlePair argument_type;
  bool operator()(const ParticlePair &c) {
    out_.push_back(c);
    return true;
  }
  void check_contains(const ParticlePair &c) const {
    if (std::find(out_.begin(), out_.end(), c) == out_.end()
        && std::find(out_.begin(), out_.end(), ParticlePair(c[1], c[0]))
        == out_.end()) {
      IMP_INTERNAL_CHECK(false, "Particle pair " << c[0]->get_name()
                         << ", " << c[1]->get_name()
                         << " not found in list. Coordinates are "
                         << XYZR(c[0]) << " and " << XYZR(c[1])
                         << " and distance is " << get_distance(XYZR(c[0]),
                                                                XYZR(c[1])));
    }
  }
};


template <class PS>
struct ParticlePairSinkWithMax: public ParticlePairSink {
  double &score_;
  double max_;
  PS *ssps_;
  DerivativeAccumulator *da_;
  ParticlePairSinkWithMax(ParticlePairsTemp &out,
                          PS *ssps,
                          DerivativeAccumulator *da,
                          double &score,
                          double max):
    ParticlePairSink(out),
    score_(score),
    max_(max),
    ssps_(ssps), da_(da) {}
  bool operator()(const ParticlePair &c) {
    ParticlePairSink::operator()(c);
    double cur= ssps_->PS::evaluate(c, da_);
    max_-=cur;
    score_+=cur;
    if (max_ < 0) {
      return false;
    }
    return true;
  }
};

struct HalfParticlePairSink: public ParticlePairSink {
  Particle *p_;
  HalfParticlePairSink(ParticlePairsTemp &out,
                       Particle *p):
    ParticlePairSink(out),
    p_(p) {}
  bool operator()(Particle *c) {
    return ParticlePairSink::operator()(ParticlePair(p_, c));
  }
  void check_contains(Particle *c) const {
    ParticlePairSink::check_contains(ParticlePair(p_, c));
  }
};

template <class PS>
struct HalfParticlePairSinkWithMax: public ParticlePairSinkWithMax<PS> {
  Particle *p_;
  HalfParticlePairSinkWithMax(ParticlePairsTemp &out,
                              PS *ssps,
                              DerivativeAccumulator *da,
                              double &score,
                              double max, Particle *p):
    ParticlePairSinkWithMax<PS>(out, ssps, da, score, max),
    p_(p) {}
  bool operator()(Particle *c) {
    return ParticlePairSinkWithMax<PS>::operator()(ParticlePair(p_, c));
  }
  void check_contains(Particle *c) const {
    ParticlePairSinkWithMax<PS>::check_contains(ParticlePair(p_, c));
  }
};


struct RigidBodyRigidBodyParticlePairSink:
  public ParticlePairSink {
  ObjectKey key_;
  double dist_;
  const IMP::compatibility::map<RigidBody, Particles> &map_;
  RigidBodyRigidBodyParticlePairSink(ParticlePairsTemp &out,
                                     ObjectKey key,
                                     double dist,
                      const IMP::compatibility::map<RigidBody, Particles>
                                     &map):
    ParticlePairSink(out),
    key_(key), dist_(dist), map_(map){}
  RigidBodyHierarchy *get_hierarchy(Particle *p) const {
    RigidBody rb(p);
    return get_rigid_body_hierarchy(rb, map_.find(rb)->second, key_);
  }
  bool operator()(const ParticlePair &c) {
    IMP_LOG(VERBOSE, "Processing interesction between "
            << c[0]->get_name() << " and "
            << c[1]->get_name() << std::endl);
    fill_close_pairs(get_hierarchy(c[0]),
                     get_hierarchy(c[1]),
                     dist_, static_cast<ParticlePairSink >(*this));
    return true;
  }
  void check_contains(const ParticlePair &) const {
    // can't look for root pair, too lazy to check for actual pairs
  }
};


struct RigidBodyParticleParticlePairSink:
  public ParticlePairSink {
  ObjectKey key_;
  double dist_;
  const IMP::compatibility::map<RigidBody, Particles> &map_;
  RigidBodyParticleParticlePairSink(ParticlePairsTemp &out,
                                    ObjectKey key, double dist,
               const IMP::compatibility::map<RigidBody, Particles>
                                     &map):
    ParticlePairSink(out),
    key_(key), dist_(dist), map_(map){}
  RigidBodyHierarchy *get_hierarchy(Particle *p) const {
    RigidBody rb(p);
    return get_rigid_body_hierarchy(rb, map_.find(rb)->second, key_);
  }
  bool operator()(const ParticlePair &c) {
    IMP_LOG(VERBOSE, "Processing rb-p interesction between "
            << c[0]->get_name() << " and "
            << c[1]->get_name() << std::endl);
    HalfParticlePairSink hps(out_, c[1]);
    fill_close_particles(get_hierarchy(c[0]),
                         XYZR(c[1]),
                         dist_, hps);
    return true;
  }
  void check_contains(const ParticlePair &) const {
    // can't look for root pair, too lazy to check for actual pairs
  }
};

template <class PS>
struct RigidBodyParticlePairSinkWithMax:
  public ParticlePairSinkWithMax<PS> {
  ObjectKey key_;
  double dist_;
  const IMP::compatibility::map<RigidBody, Particles> &map_;
  RigidBodyParticlePairSinkWithMax(ParticlePairsTemp &out,
                                   PS *ssps,
                                   DerivativeAccumulator *da,
                                   double &score,
                                   double max,
                                   ObjectKey key,
                                   double dist,
               const IMP::compatibility::map<RigidBody, Particles>
                                     &map):
    ParticlePairSinkWithMax<PS>(out, ssps, da, score, max),
    key_(key), dist_(dist), map_(map){}
  RigidBodyHierarchy *get_hierarchy(Particle *p) const {
    RigidBody rb(p);
    return get_rigid_body_hierarchy(rb, map_.find(rb)->second, key_);
  }
  void check_contains(const ParticlePair &c) const {
    // can't look for root pair, too lazy to check for actual pairs
  }
};

template <class PS>
struct RigidBodyRigidBodyParticlePairSinkWithMax:
  public RigidBodyParticlePairSinkWithMax<PS> {
  typedef RigidBodyParticlePairSinkWithMax<PS> P;
  RigidBodyRigidBodyParticlePairSinkWithMax
  (ParticlePairsTemp &out,
   PS *ssps,
   DerivativeAccumulator *da,
   double &score,
   double max,
   ObjectKey key, double dist,
   const IMP::compatibility::map<RigidBody, Particles>
   &map): P(out, ssps, da,
            score, max,
            key, dist, map)
  {}
  bool operator()(const ParticlePair &c) {
    fill_close_pairs(RigidBodyParticlePairSinkWithMax<PS>::get_hierarchy(c[0]),
                     RigidBodyParticlePairSinkWithMax<PS>::get_hierarchy(c[1]),
                     P::dist_,
                     static_cast<ParticlePairSinkWithMax<PS> >(*this));
    return P::max_ > 0;
  }
  void check_contains(const ParticlePair &) const {
    // can't look for root pair, too lazy to check for actual pairs
  }
};

template <class PS>
struct RigidBodyParticleParticlePairSinkWithMax:
  public RigidBodyParticlePairSinkWithMax<PS> {
  typedef RigidBodyParticlePairSinkWithMax<PS> P;
  RigidBodyParticleParticlePairSinkWithMax
  (ParticlePairsTemp &out,
   PS *ssps,
   DerivativeAccumulator *da,
   double &score,
   double max,
   ObjectKey key, double dist,
   const IMP::compatibility::map<RigidBody, Particles>
   &map): P(out, ssps, da,
            score, max, key, dist, map)
  {}
  bool operator()(const ParticlePair &c) {
    fill_close_particles(P::get_hierarchy(c[0]),
                         XYZR(c[1]),
                         P::dist_, HalfParticlePairSinkWithMax<PS>(P::out_,
                                                                P::ssps_,
                                                                P::da_,
                                                                P::score_,
                                                                P::max_, c[1]));
    return P::max_ > 0;
  }
  void check_contains(const ParticlePair &) const {
    // can't look for root pair, too lazy to check for actual pairs
  }
};

IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_SINKS_H */
