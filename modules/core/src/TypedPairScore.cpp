/**
 *  \file TypedPairScore.cpp
 *  \brief Delegate to another PairScore depending on particle types.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#include <IMP/core/TypedPairScore.h>

IMPCORE_BEGIN_NAMESPACE

Float TypedPairScore::evaluate(const ParticlePair &p,
                               DerivativeAccumulator *da) const
{
  PairScore *ps= get_pair_score(p);
  if (!ps) {
    if (!allow_invalid_types_) {
      IMP_THROW("Attempt to evaluate TypedPairScore on "
                "particles with invalid types ("
                << p[0]->get_value(typekey_) << ", "
                << p[1]->get_value(typekey_) << ")",
                ValueException);
    } else {
      return 0.0;
    }
  } else {
    return ps->evaluate(p, da);
  }
}

PairScore *TypedPairScore::get_pair_score(const ParticlePair &p) const {
  if (!p[0]->has_attribute(typekey_)) {
    set_particle_type(p[0]);
  }
  if (!p[1]->has_attribute(typekey_)) {
    set_particle_type(p[1]);
  }
  Int atype = p[0]->get_value(typekey_);
  Int btype = p[1]->get_value(typekey_);

  ScoreMap::const_iterator psit =
      score_map_.find(std::pair<Int,Int>(std::min(atype, btype),
                                         std::max(atype, btype)));
  if (psit == score_map_.end()) {
    return NULL;
  } else {
    PairScore *ps = psit->second.get();
    return ps;
  }
}

TypedPairScore::TypedPairScore(IntKey typekey, bool allow_invalid_types)
      : typekey_(typekey), score_map_(),
    allow_invalid_types_(allow_invalid_types) {}


ParticlesList TypedPairScore
::get_interacting_particles(const ParticlePair &p) const {
  return ParticlesList(1, get_input_particles(p));
}

// should pass it off, fix later
ParticlesTemp TypedPairScore::get_input_particles(const ParticlePair &p) const {
  ParticlesTemp ret(2);
  ret[0]=p[0];
  ret[1]=p[1];
  return ret;
}

ContainersTemp
TypedPairScore::get_input_containers(const ParticlePair &p) const {
  return ContainersTemp();
}

bool TypedPairScore::get_is_changed(const ParticlePair&p) const {
  if (p[0]->get_is_changed() || p[1]->get_is_changed()) return true;
  PairScore *ps= get_pair_score(p);
  if (!ps) return false;
  else return ps->get_is_changed(p);
}


void TypedPairScore::show(std::ostream &out) const
{
  out << "TypedPairScore with type key " << typekey_;
}

IMPCORE_END_NAMESPACE
