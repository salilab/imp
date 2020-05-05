/**
 *  \file AttributeDistancePairScore.cpp
 *  \brief A score based on the difference between the attributes
 *  of two particles
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 */

#include <IMP/spb/AttributeDistancePairScore.h>

#include <IMP/Particle.h>
#include <IMP/UnaryFunction.h>

#include <boost/tuple/tuple.hpp>

IMPSPB_BEGIN_NAMESPACE

AttributeDistancePairScore::AttributeDistancePairScore(UnaryFunction *f,
                                                       FloatKey k)
    : f_(f), k_(k) {}
/*
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
*/

void AttributeDistancePairScore::show(std::ostream &out) const {
  out << "function is " << *f_ << " on " << k_;
}

Float AttributeDistancePairScore::evaluate_index(
    IMP::Model *m, const IMP::ParticleIndexPair &pip,
    DerivativeAccumulator *da) const {
  // turn on logging for this method
  IMP_OBJECT_LOG;

  /*IMP::ParticlesTemp ps;
  ps.push_back(m->get_particle(pip[0]));
  ps.push_back(m->get_particle(pip[1]));

 if (da) {
   // derivatives are requested
   Float v,d;
   boost::tie(v,d) = f_->evaluate_with_derivative(
       ps[0]->get_value(k_)-ps[1]->get_value(k_));

   ps[0]->add_to_derivative(k_, d, *da);
   ps[1]->add_to_derivative(k_, -d, *da);

   return v;
 } else {
    return f_->evaluate(ps[0]->get_value(k_)-ps[1]->get_value(k_));
 } */

  if (da) {
    // derivatives are requested
    Float v, d;
    boost::tie(v, d) =
        f_->evaluate_with_derivative(m->get_particle(pip[0])->get_value(k_) -
                                     m->get_particle(pip[1])->get_value(k_));

    m->get_particle(pip[0])->add_to_derivative(k_, d, *da);
    m->get_particle(pip[1])->add_to_derivative(k_, d, *da);

    return v;
  } else {
    return f_->evaluate(m->get_particle(pip[0])->get_value(k_) -
                        m->get_particle(pip[1])->get_value(k_));
  }
}

ModelObjectsTemp AttributeDistancePairScore::do_get_inputs(
    IMP::Model *m, const IMP::ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}

IMPSPB_END_NAMESPACE
