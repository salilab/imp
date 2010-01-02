/**
 *  \file BoundingBox3DSingletonScore.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/BoundingBox3DSingletonScore.h"
#include "IMP/core/XYZ.h"
#include <IMP/core/internal/evaluate_distance_pair_score.h>

#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

BoundingBox3DSingletonScore
::BoundingBox3DSingletonScore(UnaryFunction *f,
                            const algebra::BoundingBox3D &bb ): f_(f), bb_(bb){
  IMP_USAGE_CHECK(std::abs(f_->evaluate(0)) <.1,
                  "The unary function should return "
            " 0 when passed a value of 0. Not " << f_->evaluate(0),
            InvalidStateException);
}

double BoundingBox3DSingletonScore::evaluate(Particle *p,
                                           DerivativeAccumulator *da) const {
  core::XYZ d(p);
  algebra::Vector3D cp;
  bool outside=false;
  for (unsigned int i=0; i< 3; ++i) {
    if (bb_.get_corner(0)[i] > d.get_coordinate(i)) {
      cp[i]=bb_.get_corner(0)[i];
      outside=true;
    } else if (bb_.get_corner(1)[i] < d.get_coordinate(i)) {
      cp[i]=bb_.get_corner(1)[i];
      outside=true;
    } else {
      cp[i]= d.get_coordinate(i);
    }
  }
  if (outside) {
    algebra::Vector3D deriv;
    double v= internal::compute_distance_pair_score(d.get_coordinates()-cp,
                                                    f_,&deriv,
                                                    boost::lambda::_1);
    if (da) {
      d.add_to_derivatives(deriv, *da);
    }
    return v;
  } else {
    return 0;
  }
}


ParticlesList
BoundingBox3DSingletonScore::get_interacting_particles(Particle*) const {
  return ParticlesList();
}

ParticlesTemp
BoundingBox3DSingletonScore::get_input_particles(Particle* p) const {
  return ParticlesTemp(1, p);
}

ContainersTemp
BoundingBox3DSingletonScore::get_input_containers(Particle* p) const {
  return ContainersTemp();
}

void BoundingBox3DSingletonScore::show(std::ostream &out) const {
  out << "BoundingBox3DSingletonScore" << std::endl;
}

IMPCORE_END_NAMESPACE
