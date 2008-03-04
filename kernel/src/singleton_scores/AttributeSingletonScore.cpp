/**
 *  \file AttributeSingletonScore.cpp
 *  \brief A score based on an unmodified attribute value.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/singleton_scores/AttributeSingletonScore.h"
#include "IMP/UnaryFunction.h"
#include "IMP/Particle.h"

namespace IMP
{

AttributeSingletonScore::AttributeSingletonScore(FloatKey k,
                                                 UnaryFunction *f): f_(f),
                                                                    k_(k){}

Float AttributeSingletonScore::evaluate(Particle *b,
                                        DerivativeAccumulator *da)
{
  if (da) {
    Float d;
    float r= (*f_)(b->get_value(k_), d);
    b->add_to_derivative(k_, d, *da);
    return r;
  } else {
    return (*f_)(b->get_value(k_));
  }
}

void AttributeSingletonScore::show(std::ostream &out) const
{
  out << "AttributeSingletonScore using ";
  f_->show(out);
  out << " on " << k_;
}

} // namespace IMP
