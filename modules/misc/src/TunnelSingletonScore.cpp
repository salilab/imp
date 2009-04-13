/**
 *  \file TunnelSingletonScore.cpp
 *  \brief Score particles with respect to a tunnel.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/misc/TunnelSingletonScore.h>

#include <IMP/core/XYZDecorator.h>

#include <boost/tuple/tuple.hpp>

IMPMISC_BEGIN_NAMESPACE

TunnelSingletonScore::TunnelSingletonScore(UnaryFunction *f, FloatKey r) :
    f_(f), rk_(r)
{
}



Float TunnelSingletonScore::evaluate(Particle *p,
                                     DerivativeAccumulator *accum) const
{
  core::XYZDecorator d(p);
  Float radius=0;
  if (rk_ != FloatKey() && p->has_attribute(rk_)) {
    radius = p->get_value(rk_);
  }
  IMP_LOG(VERBOSE, "Tunnel singleton score on particle " << d << std::endl);
  Float dist= tr_.get_distance(d.get_coordinates());
  if (dist > radius) {
    IMP_LOG(VERBOSE, "Particle is too far: " << radius << " < " << dist
            << std::endl);
    return 0;
  } else {
    Float adist= radius-dist;
    if (accum) {
      algebra::Vector3D v
        = tr_.get_displacement_unit_vector(d.get_coordinates());

      DerivativePair val= f_->evaluate_with_derivative(-adist);
      IMP_LOG(VERBOSE, "Result of eval on " << adist << " "
              << radius << " " << dist
              << " " << v << " " << val.first << " " << val.second
              << std::endl);
      algebra::Vector3D dv= v*val.second;
      d.add_to_derivatives(dv, *accum);
      return val.first;
    } else {
      Float val= f_->evaluate(-adist);
      return val;
    }
  }
}

void TunnelSingletonScore::show(std::ostream& out) const
{
  out << "Tunnel score :" << *f_ << std::endl;
}

IMPMISC_END_NAMESPACE
