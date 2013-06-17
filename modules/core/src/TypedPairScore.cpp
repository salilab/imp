/**
 *  \file TypedPairScore.cpp
 *  \brief Delegate to another PairScore depending on particle types.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/core/TypedPairScore.h>

IMPCORE_BEGIN_NAMESPACE

Float TypedPairScore::evaluate_index(Model *m,
                                     const ParticleIndexPair &pip,
                                     DerivativeAccumulator *da) const {
  ParticlePair p(m->get_particle(pip[0]),
                  m->get_particle(pip[1]));
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
    return ps->evaluate_index(m, pip, da);
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
ModelObjectsTemp TypedPairScore::do_get_inputs(Model *m,
                                               const ParticleIndexes &pis)
    const {
  return IMP::kernel::get_particles(m, pis);
}

IMPCORE_END_NAMESPACE
