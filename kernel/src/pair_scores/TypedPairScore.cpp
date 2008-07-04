/**
 *  \file TypedPairScore.cpp
 *  \brief Delegate to another PairScore depending on particle types.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/pair_scores/TypedPairScore.h"

namespace IMP
{

Float TypedPairScore::evaluate(Particle *a, Particle *b,
                               DerivativeAccumulator *da)
{
  if (!a->has_attribute(typekey_)) {
    set_particle_type(a);
  }
  if (!b->has_attribute(typekey_)) {
    set_particle_type(b);
  }
  Int atype = a->get_value(typekey_);
  Int btype = b->get_value(typekey_);

  ScoreMap::iterator psit =
      score_map_.find(std::pair<Int,Int>(std::min(atype, btype),
                                         std::max(atype, btype)));
  if (psit == score_map_.end()) {
    IMP_check(allow_invalid_types_, "Attempt to evaluate TypedPairScore on "
              "particles with invalid types (" << atype << ", " << btype << ")",
               ValueException);
    return 0.;
  } else {
    PairScore *ps = psit->second.get();
    return ps->evaluate(a, b, da);
  }
}


void TypedPairScore::show(std::ostream &out) const
{
  out << "TypedPairScore with type key " << typekey_;
}

} // namespace IMP
