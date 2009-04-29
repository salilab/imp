/**
 *  \file TypedPairScore.cpp
 *  \brief Delegate to another PairScore depending on particle types.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP/core/TypedPairScore.h>

IMPCORE_BEGIN_NAMESPACE

Float TypedPairScore::evaluate(Particle *a, Particle *b,
                               DerivativeAccumulator *da) const
{
  if (!a->has_attribute(typekey_)) {
    set_particle_type(a);
  }
  if (!b->has_attribute(typekey_)) {
    set_particle_type(b);
  }
  Int atype = a->get_value(typekey_);
  Int btype = b->get_value(typekey_);

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
    return ps->evaluate(a, b, da);
  }
}


void TypedPairScore::show(std::ostream &out) const
{
  out << "TypedPairScore with type key " << typekey_;
}

IMPCORE_END_NAMESPACE
