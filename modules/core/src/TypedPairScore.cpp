/**
 *  \file TypedPairScore.cpp
 *  \brief Delegate to another PairScore depending on particle types.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/core/TypedPairScore.h>

IMPCORE_BEGIN_NAMESPACE

Float TypedPairScore::evaluate(const ParticlePair &p,
                               DerivativeAccumulator *da) const {
  PairScore *ps = get_pair_score(p);
  if (!ps) {
    if (!allow_invalid_types_) {
      IMP_THROW(
          "Attempt to evaluate TypedPairScore on "
          "particles with invalid types (" << p[0]->get_value(typekey_) << ", "
                                           << p[1]->get_value(typekey_) << ")",
          ValueException);
    } else {
      return 0.0;
    }
  } else {
    return ps->evaluate_index(
        p[0]->get_model(),
        ParticleIndexPair(p[0]->get_index(), p[1]->get_index()), da);
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

  ScoreMap::const_iterator psit = score_map_.find(
      std::pair<Int, Int>(std::min(atype, btype), std::max(atype, btype)));
  if (psit == score_map_.end()) {
    return nullptr;
  } else {
    PairScore *ps = psit->second.get();
    return ps;
  }
}

TypedPairScore::TypedPairScore(IntKey typekey, bool allow_invalid_types)
    : typekey_(typekey),
      score_map_(),
      allow_invalid_types_(allow_invalid_types) {}

// should pass it off, fix later
ParticlesTemp TypedPairScore::get_input_particles(Particle *p) const {
  ParticlesTemp ret(1, p);
  return ret;
}

ContainersTemp TypedPairScore::get_input_containers(Particle *) const {
  return ContainersTemp();
}

void TypedPairScore::do_show(std::ostream &out) const {
  out << "key " << typekey_ << std::endl;
}

IMPCORE_END_NAMESPACE
