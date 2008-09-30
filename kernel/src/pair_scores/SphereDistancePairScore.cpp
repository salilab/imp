/**
 *  \file SphereDistancePairScore.cpp
 *  \brief A score on the distance between the surfaces of two spheres.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/pair_scores/SphereDistancePairScore.h"
#include "IMP/UnaryFunction.h"
#include "IMP/pair_scores/DistancePairScore.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/internal/evaluate_distance_pair_score.h"
#include "boost/lambda/lambda.hpp"

IMP_BEGIN_NAMESPACE

SphereDistancePairScore::SphereDistancePairScore(UnaryFunction *f,
                                                 FloatKey radius) :
    f_(f), radius_(radius)
{
}

struct Shift
{
  Float s_;
  Shift(Float s): s_(s){}
  Float operator()(Float t) const {return t-s_;}
};

Float SphereDistancePairScore::evaluate(Particle *a, Particle *b,
                                        DerivativeAccumulator *da) const
{
  IMP_check(a->has_attribute(radius_), "Particle " << a->get_index()
            << "missing radius in SphereDistancePairScore",
            ValueException);
  IMP_check(b->has_attribute(radius_), "Particle " << b->get_index()
            << "missing radius in SphereDistancePairScore",
            ValueException);
  Float ra = a->get_value(radius_);
  Float rb = b->get_value(radius_);
  return internal::evaluate_distance_pair_score(XYZDecorator(a),
                                                XYZDecorator(b),
                                                da, f_.get(),
                                                boost::lambda::_1-(ra+rb));
}

void SphereDistancePairScore::show(std::ostream &out) const
{
  out << "SphereDistancePairScore using ";
  f_->show(out);
}

IMP_END_NAMESPACE
