/**
 *  \file AttributeDistancePairScore.cpp
 *  \brief A score based on the difference between the attributes
 *  of two particles
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#include <IMP/membrane/AttributeDistancePairScore.h>

#include <IMP/UnaryFunction.h>
#include <IMP/Particle.h>

#include <boost/tuple/tuple.hpp>

IMPMEMBRANE_BEGIN_NAMESPACE

AttributeDistancePairScore::AttributeDistancePairScore(UnaryFunction *f,
                                                 FloatKey k): f_(f),
                                                              k_(k){}

Float AttributeDistancePairScore::evaluate(const ParticlePair &p,
                                        DerivativeAccumulator *da) const
{
  if (da) {
    Float v, d;
    boost::tie(v,d) = f_->evaluate_with_derivative(p[0]->get_value(k_)
                                                  -p[1]->get_value(k_));
    p[0]->add_to_derivative(k_, d, *da);
    p[1]->add_to_derivative(k_, -d, *da);
    return v;
  } else {
    return f_->evaluate(p[0]->get_value(k_)-p[1]->get_value(k_));
  }
}

void AttributeDistancePairScore::do_show(std::ostream &out) const
{
  out << "function is " << *f_ << " on " << k_;
}

IMPMEMBRANE_END_NAMESPACE
