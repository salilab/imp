/**
 *  \file SphereDistancePairScore.cpp
 *  \brief A score on the distance between the surfaces of two spheres.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include "IMP/isd/RegularRepulsiveDistancePairScore.h"
#include <math.h>
#include <IMP/atom/LennardJones.h>
#include <IMP/atom/smoothing_functions.h>

IMPISD_BEGIN_NAMESPACE

//RegularRepulsiveDistancePairScore::RegularRepulsiveDistancePairScore(double dmin, double e) :
//    dmin_(dmin), e_(e) {}



inline double RegularRepulsiveDistancePairScore::evaluate(const ParticlePair &p,
                            DerivativeAccumulator *da) const {
  core::XYZR d0(p[0]), d1(p[1]);			    
//  atom::LennardJones lj0(p[0]);
//  atom::LennardJones lj1(p[1]);
  algebra::VectorD<3> delta = d0.get_coordinates() - d1.get_coordinates();
//  for (int i = 0; i < 3; ++i) {
//    delta[i] = lj0.get_coordinate(i) - lj1.get_coordinate(i);
//  }
  double distsqr= delta.get_squared_magnitude();
  double dist=std::sqrt(distsqr);
  double rmin = d0.get_radius() + d1.get_radius();
//  double shifted_distance = distance - target;
//  if (shifted_distance > 0) return 0;

  double rrmin30 = pow ( dist / rmin , 30 );
  double score = e_ / ( 1.0 + rrmin30 );
  
  
  if (da) {
    DerivativePair d =
            (*smoothing_function_)(score,
                                   -30.0 * e_ * rrmin30 / dist / (rrmin30 + 1.0 ) / (rrmin30 + 1.0 ) ,
                                   dist);
    algebra::VectorD<3> deriv = d.second * delta / dist;
    d0.add_to_derivatives(deriv, *da);
    d1.add_to_derivatives(-deriv, *da);
    return d.first;
  } else {
    return (*smoothing_function_)(score, dist);
  }
}

void RegularRepulsiveDistancePairScore::do_show(std::ostream &out) const {
   out << "e=" << e_ << std::endl;
  }

IMPISD_END_NAMESPACE
