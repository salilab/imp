/**
 *  \file SphereDistancePairScore.cpp
 *  \brief A score on the distance between the surfaces of two spheres.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/pair_scores/SphereDistancePairScore.h"
#include "IMP/UnaryFunction.h"
#include "IMP/pair_scores/DistancePairScore.h"

namespace IMP
{

SphereDistancePairScore::SphereDistancePairScore(UnaryFunction *f,
                                                 FloatKey radius) :
    f_(f), radius_(radius)
{
}

Float SphereDistancePairScore::evaluate(Particle *a, Particle *b,
                                        DerivativeAccumulator *da)
{
  IMP_check(a->has_attribute(radius_), "Particle " << a->get_index() 
            << "missing radius in SphereDistancePairScore",
            ValueException("Missing radius"));
  IMP_check(b->has_attribute(radius_), "Particle " << b->get_index() 
            << "missing radius in SphereDistancePairScore",
            ValueException("Missing radius"));
  Float ra = a->get_value(radius_);
  Float rb = b->get_value(radius_);
  return internal::evaluate_distance_pair_score(a,b, da, f_.get(), 
                                                ra+rb, 1);
}

void SphereDistancePairScore::show(std::ostream &out) const
{
  out << "SphereDistancePairScore using ";
  f_->show(out);
}

} // namespace IMP
