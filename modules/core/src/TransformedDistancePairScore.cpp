/**
 *  \file TransformedDistancePairScore.cpp
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include <IMP/core/TransformedDistancePairScore.h>
#include <IMP/core/XYZDecorator.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>

#include <IMP/UnaryFunction.h>
#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

TransformedDistancePairScore
::TransformedDistancePairScore(UnaryFunction *f,
                               const Transformation3D &t): f_(f)
{
  set_transformation(t);
}

struct TransformParticle
{
  Vector3D tv_;
  const Rotation3D &ri_;
  XYZDecorator d_;
  TransformParticle(const Transformation3D &t,
                    const Rotation3D &r,
                    Particle *p): ri_(r), d_(p){
    tv_= t.transform(d_.get_coordinates());
  }

  Float get_coordinate(unsigned int i) const {
    return tv_[i];
  }

  void add_to_coordinates_derivative(const Vector3D& f,
                                     DerivativeAccumulator &da) {
    IMP_LOG(VERBOSE, "Incoming deriv is " << f << std::endl);
    Vector3D r= ri_.rotate(f);
    IMP_LOG(VERBOSE, "Transformed deriv is " << r << std::endl);
    d_.add_to_coordinates_derivative(r, da);
  }
};

Float TransformedDistancePairScore::evaluate(Particle *a, Particle *b,
                                             DerivativeAccumulator *da) const
{
  TransformParticle tb(t_, ri_, b);
  IMP_LOG(VERBOSE, "Transformed particle is "
          << tb.get_coordinate(0) << " " << tb.get_coordinate(1)
          << " " << tb.get_coordinate(2) << std::endl);
  Float ret= internal::evaluate_distance_pair_score(XYZDecorator(a),
                                                    tb,
                                                    da, f_.get(),
                                                    boost::lambda::_1);
  return ret;
}


void TransformedDistancePairScore::set_transformation(const Transformation3D &t)
{
  ri_= t.get_rotation().get_inverse();
  t_=t;
}


void TransformedDistancePairScore::show(std::ostream &out) const
{
  out << "TransformedDistancePairScore using ";
  f_->show(out);
  out << " and \n" << t_ << std::endl;
}

IMPCORE_END_NAMESPACE
