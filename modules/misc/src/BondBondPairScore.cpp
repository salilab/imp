/**
 *  \file BondBondPairScore.cpp
 *  \brief Score on the cylinerCyliner between a pair of particles.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include "IMP/misc/BondBondPairScore.h"
#include "IMP/core/XYZ.h"
#include "IMP/constants.h"
#include "IMP/core/internal/evaluate_distance_pair_score.h"
#include "IMP/atom/bond_decorators.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/internal/shortest_segment.h>
#include <IMP/Particle.h>
#include <boost/lambda/lambda.hpp>

IMPMISC_BEGIN_NAMESPACE


BondBondPairScore
::BondBondPairScore(UnaryFunction *f,
                    FloatKey volume): f_(f),
                                      volume_(volume)
{
}

namespace {

  algebra::Segment3D get_endpoints(Particle *p)
  {
    atom::Bond b(p);
    Particle* p0= b.get_bonded(0).get_particle();
    Particle* p1= b.get_bonded(1).get_particle();
    IMP::core::XYZ d0(p0);
    IMP::core::XYZ d1(p1);
    return algebra::Segment3D(d0.get_coordinates(), d1.get_coordinates());
}


  algebra::Vector3D get_difference(const algebra::Segment3D &s0,
                                   const algebra::Segment3D &s1)
  {
    algebra::Segment3D s= algebra::internal::shortest_segment(s0, s1);
    return s.get_point(0)-s.get_point(1);
  }

}


Float BondBondPairScore::get_offset(Particle *p,
                                    Float d) const
{
  if (volume_ != FloatKey()) {
    Float v= p->get_value(volume_);
    /*
      v= d*pi*r^2
      r= sqrt(v/ d*pi)
     */
    Float r= std::sqrt(v/ (d * PI));
    return r;
  } else {
    return 0;
  }
}


Float BondBondPairScore::evaluate(Particle *a, Particle *b,
                                          DerivativeAccumulator *da) const
{

  algebra::Segment3D ep[2]= {get_endpoints(a), get_endpoints(b)};
  Float d[2];
  for (int i=0; i< 2; ++i) {
    d[i]= (ep[i].get_point(0)-ep[i].get_point(1)).get_magnitude();
  }

  Float o[2]= {get_offset(a, d[0]),
               get_offset(b, d[1])};
  algebra::Vector3D diff= get_difference(ep[0], ep[1]);

  algebra::Vector3D deriv;
  Float score= IMP::core::internal::compute_distance_pair_score(diff, f_,
                                                     (da? &deriv : NULL),
                                                     boost::lambda::_1
                                                     - o[0] - o[1]);
  if (da) {
    IMP::core::XYZ xyzd[2]={IMP::core::XYZ(a),
                                     IMP::core::XYZ(b)};
    xyzd[0].add_to_derivatives(deriv, *da);
    xyzd[1].add_to_derivatives(-deriv, *da);
  }

  return score;
}

void BondBondPairScore::show(std::ostream &out) const
{
  out << "BondBondPairScore using ";
  f_->show(out);
}

IMPMISC_END_NAMESPACE
