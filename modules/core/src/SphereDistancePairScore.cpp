/**
 *  \file SphereDistancePairScore.cpp
 *  \brief A score on the distance between the surfaces of two spheres.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/XYZR.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>

#include <IMP/UnaryFunction.h>
#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

SphereDistancePairScore::SphereDistancePairScore(UnaryFunction *f,
                                                 FloatKey radius) :
    f_(f), radius_(radius)
{
}
namespace {
struct Shift
{
  Float s_;
  Shift(Float s): s_(s){}
  Float operator()(Float t) const {return t-s_;}
};
}

Float SphereDistancePairScore::evaluate(const ParticlePair &p,
                                        DerivativeAccumulator *da) const
{
  IMP_USAGE_CHECK(p[0]->has_attribute(radius_), "Particle " << p[0]->get_name()
            << "missing radius in SphereDistancePairScore");
  IMP_USAGE_CHECK(p[1]->has_attribute(radius_), "Particle " << p[1]->get_name()
            << "missing radius in SphereDistancePairScore");
  Float ra = p[0]->get_value(radius_);
  Float rb = p[1]->get_value(radius_);
  return internal::evaluate_distance_pair_score(XYZ(p[0]),
                                                XYZ(p[1]),
                                                da, f_.get(),
                                                boost::lambda::_1-(ra+rb));
}



void SphereDistancePairScore::do_show(std::ostream &out) const
{
  out << "function " << *f_ << std::endl;
}







NormalizedSphereDistancePairScore
::NormalizedSphereDistancePairScore(UnaryFunction *f,
                                    FloatKey radius) :
    f_(f), radius_(radius)
{
}


Float NormalizedSphereDistancePairScore::evaluate(const ParticlePair &p,
                                        DerivativeAccumulator *da) const
{
  IMP_USAGE_CHECK(p[0]->has_attribute(radius_), "Particle " << p[0]->get_name()
            << "missing radius in NormalizedSphereDistancePairScore");
  IMP_USAGE_CHECK(p[1]->has_attribute(radius_), "Particle " << p[1]->get_name()
            << "missing radius in NormalizedSphereDistancePairScore");
  Float ra = p[0]->get_value(radius_);
  Float rb = p[1]->get_value(radius_);
  Float mr= std::min(ra, rb);
  // lambda is inefficient due to laziness
  return internal::evaluate_distance_pair_score(XYZ(p[0]),
                                                XYZ(p[1]),
                                                da, f_.get(),
                                         boost::lambda::_1/mr-(ra+rb)/mr);
}


void NormalizedSphereDistancePairScore::do_show(std::ostream &out) const
{
  out << "function " << *f_ << std::endl;
}

WeightedSphereDistancePairScore::WeightedSphereDistancePairScore(
  UnaryFunction *f,FloatKey weight,FloatKey radius):
  f_(f), radius_(radius), weight_(weight){
}

Float WeightedSphereDistancePairScore::evaluate(const ParticlePair &p,
                                       DerivativeAccumulator *da) const
{
  IMP_USAGE_CHECK(p[0]->has_attribute(radius_), "Particle " << p[0]->get_name()
            << "missing radius in WeightedSphereDistancePairScore");
  IMP_USAGE_CHECK(p[1]->has_attribute(radius_), "Particle " << p[1]->get_name()
            << "missing radius in WeightedSphereDistancePairScore");
  IMP_USAGE_CHECK(p[0]->has_attribute(weight_), "Particle " << p[0]->get_name()
            << "missing weight in WeightedSphereDistancePairScore");
  IMP_USAGE_CHECK(p[1]->has_attribute(weight_), "Particle " << p[1]->get_name()
            << "missing weight in WeightedSphereDistancePairScore");
  Float ra = p[0]->get_value(radius_);
  Float rb = p[1]->get_value(radius_);
  Float wa = p[0]->get_value(weight_);
  Float wb = p[1]->get_value(weight_);
  // lambda is inefficient due to laziness
  return internal::evaluate_distance_pair_score(
                                    XYZ(p[0]),
                                    XYZ(p[1]),
                                    da, f_.get(),
                                    (boost::lambda::_1-(ra+rb))*(wa+wb));
}

void WeightedSphereDistancePairScore::do_show(std::ostream &out) const
{
  out << "function " << *f_ << std::endl;
}

double SoftSpherePairScore::evaluate(const ParticlePair &p,
                                     DerivativeAccumulator *da) const {
  XYZR d0(p[0]), d1(p[1]);
  algebra::VectorD<3> delta;
  for (int i = 0; i < 3; ++i) {
    delta[i] = d0.get_coordinate(i) - d1.get_coordinate(i);
  }
  static const double MIN_DISTANCE = .00001;
  double distance2= delta.get_squared_magnitude();
  if (distance2 > square(d0.get_radius()+d1.get_radius())) return 0;
  double distance=std::sqrt(distance2);
  double shifted_distance = distance- d0.get_radius()-d1.get_radius();
  double score= .5*k_*square(shifted_distance);
  if (!da || distance < MIN_DISTANCE) return score;
  double deriv= k_*shifted_distance;
  algebra::Vector3D uv= delta/distance;
  if (da) {
    d0.add_to_derivatives(uv*deriv, *da);
    d1.add_to_derivatives(-uv*deriv, *da);
  }

  return score;
}
void SoftSpherePairScore::do_show(std::ostream &out) const {
  out << "k=" << k_ << std::endl;
}

IMPCORE_END_NAMESPACE
