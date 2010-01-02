/**
 *  \file TypedPairScore.cpp
 *  \brief Delegate to another PairScore depending on particle types.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP/core/TypedPairScore.h>

IMPCORE_BEGIN_NAMESPACE

Float TypedPairScore::evaluate(const ParticlePair &p,
                               DerivativeAccumulator *da) const
{
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
    if (!allow_invalid_types_) {
      std::ostringstream oss;
      oss << "Attempt to evaluate TypedPairScore on "
          "particles with invalid types (" << atype << ", " << btype << ")";
      throw ValueException(oss.str().c_str());
    } else {
      return 0.;
    }
  } else {
    PairScore *ps = psit->second.get();
    return ps->evaluate(p, da);
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


void TypedPairScore::show(std::ostream &out) const
{
  out << "TypedPairScore with type key " << typekey_;
}

IMPCORE_END_NAMESPACE
