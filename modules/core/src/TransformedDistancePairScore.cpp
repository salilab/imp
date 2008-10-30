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

static void inverse(float r00, float r01, float r02,
                    float r10, float r11, float r12,
                    float r20, float r21, float r22,
                    Vector3D *inv)
{
  Float t4 = r00*r11;
  Float t6 = r00*r12;
  Float t8 = r01*r10;
  Float t10 = r02*r10;
  Float t12 = r01*r20;
  Float t14 = r02*r20;
  Float t17i = (t4*r22-t6*r21-t8*r22+t10*r21+t12*r12-t14*r11);
  IMP_check(t17i != 0, "Singular matrix", ValueException);
  Float t17= 1.0/t17i;
  inv[0][0] = (r11*r22-r12*r21)*t17;
  inv[0][1] = -(r01*r22-r02*r21)*t17;
  inv[0][2] = (r01*r12-r02*r11)*t17;
  inv[1][0] = -(r10*r22-r12*r20)*t17;
  inv[1][1] = (r00*r22-t14)*t17;
  inv[1][2] = -(t6-t10)*t17;
  inv[2][0] = (r10*r21-r11*r20)*t17;
  inv[2][1] = -(r00*r21-t12)*t17;
  inv[2][2] = (t4-t8)*t17;
}

TransformedDistancePairScore
::TransformedDistancePairScore(UnaryFunction *f): f_(f),
                                                  tc_(0,0,0),
                                                  c_(0,0,0)
{
  set_rotation(1,0,0,
               0,1,0,
               0,0,1);
}


/*
  Compute R(x-c)+tc and the reverse
  dt= R(d-c)+tc-> Rt(dt-tc)+c
 */
struct TransformParticle
{
  const Vector3D *r_, *ri_;
  const Vector3D &tc_;
  const Vector3D &c_;
  XYZDecorator d_;
  TransformParticle(const Vector3D *r,
                    const Vector3D *ri,
                    const Vector3D &tc,
                    const Vector3D &c,
                    Particle *p): r_(r), ri_(ri),
                                  tc_(tc), c_(c), d_(p){}

  Float get_coordinate(unsigned int i) const {
    return (d_.get_coordinates()-c_)*r_[i] + tc_[i];
  }

  void add_to_coordinates_derivative(const Vector3D& f,
                                     DerivativeAccumulator &da) {
    Vector3D r(f*ri_[0],
               f*ri_[1],
               f*ri_[2]);
    d_.add_to_coordinates_derivative(r, da);
  }
};

Float TransformedDistancePairScore::evaluate(Particle *a, Particle *b,
                                             DerivativeAccumulator *da) const
{
  TransformParticle tb(r_,ri_, tc_,c_,b);
  IMP_LOG(VERBOSE, "Transformed particle is "
          << tb.get_coordinate(0) << " " << tb.get_coordinate(1)
          << " " << tb.get_coordinate(2) << std::endl);
  return internal::evaluate_distance_pair_score(XYZDecorator(a),
                                                tb,
                                                da, f_.get(),
                                                boost::lambda::_1);
}


void TransformedDistancePairScore::set_rotation(float r00, float r01, float r02,
                                                float r10, float r11, float r12,
                                                float r20, float r21, float r22)
{
  // do in two steps in case inverse throws an exception
  Vector3D v[3];
  inverse(r00, r01, r02,
          r10, r11, r12,
          r20, r21, r22,
          v);
  r_[0]= Vector3D(r00, r01, r02);
  r_[1]= Vector3D(r10, r11, r12);
  r_[2]= Vector3D(r20, r21, r22);
  ri_[0]= v[0];
  ri_[1]= v[1];
  ri_[2]= v[2];
  IMP_LOG(VERBOSE, "Rotation is \n" << r_[0] << "\n" << r_[1] << "\n"
          << r_[2] << "\nand inverse is \n"
          << ri_[0] << "\n" << ri_[1] << "\n"
          << ri_[2] << std::endl);
}

void TransformedDistancePairScore::set_translation(const Vector3D &v)
{
  tc_= v + c_;
}


void TransformedDistancePairScore::set_center(const Vector3D &c)
{
  tc_= tc_-c_;
  c_= c;
  tc_= tc_+c_;
}


void TransformedDistancePairScore::show(std::ostream &out) const
{
  out << "TransformedDistancePairScore using ";
  f_->show(out);
}

IMPCORE_END_NAMESPACE
