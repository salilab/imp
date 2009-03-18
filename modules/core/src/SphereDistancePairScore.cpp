/**
 *  \file SphereDistancePairScore.cpp
 *  \brief A score on the distance between the surfaces of two spheres.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/XYZDecorator.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>

#include <IMP/UnaryFunction.h>
#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

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
  IMP_check(a->has_attribute(radius_), "Particle " << a->get_name()
            << "missing radius in SphereDistancePairScore",
            ValueException);
  IMP_check(b->has_attribute(radius_), "Particle " << b->get_name()
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

IMPCORE_END_NAMESPACE
