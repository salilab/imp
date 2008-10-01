/**
 *  \file TunnelSingletonScore.cpp
 *  \brief Score particles with respect to a tunnel.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/misc/TunnelSingletonScore.h>
#include <IMP/decorators/XYZDecorator.h>

#include <boost/tuple/tuple.hpp>

IMPMISC_BEGIN_NAMESPACE

TunnelSingletonScore::TunnelSingletonScore(UnaryFunction *f, FloatKey r) :
    f_(f), rk_(r)
{
  coordinate_=0;
  center_=Vector3D(0,0,0);
  height_=0;
  radius_=0;
}



Float TunnelSingletonScore::evaluate(Particle *p,
                                     DerivativeAccumulator *accum) const
{
  Float ret=0;
  XYZDecorator d(p);
  Float radius=0;
  if (rk_ != FloatKey() && p->has_attribute(rk_)) {
    radius = p->get_value(rk_);
  }
  IMP_LOG(VERBOSE, "Tunnel singleton score on particle " << d << std::endl);
  Float hr= height_+radius;
  if (d.get_coordinate(coordinate_) < center_[coordinate_] + hr
      && d.get_coordinate(coordinate_) > center_[coordinate_] - hr) {
    Float sd=0;
    for (int i=1; i< 3; ++i) {
      int oc= (i+coordinate_)%3;
      sd+= square(d.get_coordinate(oc)- center_[oc]);
    }
    sd= std::sqrt(sd);
    IMP_LOG(VERBOSE, "The distance is " << sd << " and radius "
            << radius_ << std::endl);
    if (sd > radius_-radius) {
      Float rd= sd-radius_;
      Float hdu= center_[coordinate_] + height_
        - d.get_coordinate(coordinate_);
      Float hdd=  d.get_coordinate(coordinate_)
        + height_- center_[coordinate_];
      Vector3D deriv(0,0,0);
      Float score=0;
      Float deriv_scalar=0;
      /*! \todo Clean up these tests so I am not dependent on two expressions
        being the same and evaluating to the same thing */
      // look below if changed
      Float dist= -std::min(std::min(rd, hdu), hdd) - radius;
      if (accum) {
        boost::tie(score, deriv_scalar)= f_->evaluate_with_derivative(dist);
      } else {
        score= f_->evaluate(dist);
      }

      // kind if evil
      if (dist== -rd -radius) {
        Vector3D v= (d.get_vector() - center_).get_unit_vector();
        for (int i=0; i< 2; ++i) {
          int oc= (i+coordinate_+1)%3;
          deriv[oc]= v[oc]*deriv_scalar;
        }
      } else {
        // kind of evil
        if (dist == -hdu -radius) {
          deriv_scalar= -deriv_scalar;
        }
        deriv[coordinate_]= deriv_scalar;
      }

      if (accum) {
        for (unsigned int i=0; i< 3; ++i) {
          d.add_to_coordinate_derivative(i, deriv[i], *accum);
        }
      }
      ret+= score;
    } else {
      IMP_LOG(VERBOSE, "Particle " << p->get_index()
              << " is in channel" << std::endl);
    }
  } else {
    IMP_LOG(VERBOSE, "Particle " << p->get_index()
            << " is outside of slab" << std::endl);
  }
  return ret;
}

void TunnelSingletonScore::show(std::ostream& out) const
{
  out << "Tunnel score :" << *f_
      << " " << center_ << " " << height_ << " "
      << coordinate_ << std::endl;
}

IMPMISC_END_NAMESPACE
