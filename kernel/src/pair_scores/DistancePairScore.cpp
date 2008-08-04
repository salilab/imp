/**
 *  \file DistancePairScore.cpp
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/pair_scores/DistancePairScore.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/UnaryFunction.h"
#include "IMP/internal/evaluate_distance_pair_score.h"
#include <boost/lambda/lambda.hpp>

namespace IMP
{

DistancePairScore::DistancePairScore(UnaryFunction *f): f_(f){}

struct Identity
{
  Float operator()(Float t) const {return t;}
};

Float DistancePairScore::evaluate(Particle *a, Particle *b,
                                  DerivativeAccumulator *da) const
{
  return internal::evaluate_distance_pair_score(XYZDecorator(a),
                                                XYZDecorator(b),
                                                da, f_.get(),
                                                boost::lambda::_1);
}

void DistancePairScore::show(std::ostream &out) const
{
  out << "DistancePairScore using ";
  f_->show(out);
}

} // namespace IMP
