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
};

struct ParticlePairSink {
  ParticlePairsTemp &out_;
  ParticlePairSink(ParticlePairsTemp &out): out_(out){}
  typedef ParticlePair argument_type;
  bool operator()(const ParticlePair &c) {
    out_.push_back(c);
    return true;
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
};


template <class PS>
struct RigidBodyParticlePairSinkWithMax:
  public ParticlePairSinkWithMax<PS> {
  ObjectKey key_;
  RigidBodyParticlePairSinkWithMax(ParticlePairsTemp &out,
                                   PS *ssps,
                                   DerivativeAccumulator *da,
                                   double &score,
                                   double max,
                                   ObjectKey key):
    ParticlePairSinkWithMax<PS>(out, ssps, da, score, max),
    key_(key){}
  RigidBodyHierarchy *get_hierarchy(Particle *p) const {
    IMP_INTERNAL_CHECK(p->has_attribute(key_),
                       "Rigid body doesn't have tree yet");
    return dynamic_cast<RigidBodyHierarchy*>(p->get_value(key_));
  }
};

template <class PS>
struct RigidBodyRigidBodyParticlePairSinkWithMax:
  public RigidBodyParticlePairSinkWithMax<PS> {
  RigidBodyRigidBodyParticlePairSinkWithMax
  (ParticlePairsTemp &out,
   PS *ssps,
   DerivativeAccumulator *da,
   double &score,
   double max,
   ObjectKey key): RigidBodyParticlePairSinkWithMax<PS>(out, ssps, da,
                                                        score, max, key)
  {}
  bool operator()(const ParticlePair &c) {
    fill_close_pairs(RigidBodyParticlePairSinkWithMax<PS>::get_hierarchy(c[0]),
                     RigidBodyParticlePairSinkWithMax<PS>::get_hierarchy(c[1]),
                     0, static_cast<ParticlePairSinkWithMax<PS> >(*this));
    return RigidBodyParticlePairSinkWithMax<PS>::max_ > 0;
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
   ObjectKey key): P(out, ssps, da,
                     score, max, key)
  {}
  bool operator()(const ParticlePair &c) {
    fill_close_particles(P::get_hierarchy(c[0]),
                         c[1],
                         0, HalfParticlePairSinkWithMax<PS>(c[1], P::out_,
                                                        P::score_, P::max_,
                                                        P::ssps_, P::da_));
    return P::max_ > 0;
  }
};

IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_SINKS_H */
