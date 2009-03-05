/**
 *  \file BondBondPairScore.cpp
 *  \brief Score on the cylinerCyliner between a pair of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/core/BondBondPairScore.h"
#include "IMP/core/XYZDecorator.h"
#include "IMP/constants.h"
#include "IMP/core/internal/evaluate_distance_pair_score.h"
#include "IMP/atom/bond_decorators.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/Particle.h>
#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

#ifndef IMP_NO_DEPRECATED

Segment shortest_segment(const Segment &s, const algebra::Vector3D &p) {
  algebra::Vector3D vs= s.second- s.first;
  algebra::Vector3D vps= p- s.first;
  double f= vps*vs/(vs*vs);
  if (f<= 0) return Segment(s.first, p);
  else if (f>=1) return Segment(s.second, p);
  else {
    algebra::Vector3D ps= s.first + vs*f;
    return Segment(ps, p);
  }
}

/*
   Calculate the line segment PaPb that is the shortest route between
   two lines P1P2 and P3P4. Calculate also the values of mua and mub where
      Pa = P1 + mua (P2 - P1)
      Pb = P3 + mub (P4 - P3)
   Return FALSE if no solution exists.
*/
Segment shortest_segment(const Segment &sa,
                          const Segment &sb) {
  const double eps= .0001;
  algebra::Vector3D va= sa.second - sa.first;
  algebra::Vector3D vb= sb.second - sb.first;
  double ma= va*va;
  double mb= vb*vb;
  if (ma < eps) {
    if (mb < eps) {
      return Segment(sa.first, sb.first);
    } else {
      Segment sr =shortest_segment(sb, sa.first);
      return Segment(sr.second, sr.first);
    }
  } else if (mb < eps) {
    return shortest_segment(sa, sb.first);
  }

  algebra::Vector3D vfirst = sa.first- sb.first;

  IMP_LOG(VERBOSE, vfirst << " | " << va << " | " << vb << std::endl);

  double dfb = vfirst*vb;
  double dab = vb*va;
  double dfa = vfirst*va;

   double denom = ma * mb - dab * dab;
   if (std::abs(denom) < eps) {
     return Segment(sa.first, sb.first);
   }
   double numer = dfb * dab - dfa * mb;

   double fa = numer / denom;
   double fb = (dfb + dab * fa) / mb;

   /*
   denom = d2121 * d4343 - d4321 * d4321;
   numer = d1343 * d4321 - d1321 * d4343;

   *mua = numer / denom;
   *mub = (d1343 + d4321 * (*mua)) / d4343;

   pa->x = p1.x + *mua * p21.x;
   pa->y = p1.y + *mua * p21.y;
   pa->z = p1.z + *mua * p21.z;
   pb->x = p3.x + *mub * p43.x;
   pb->y = p3.y + *mub * p43.y;
   pb->z = p3.z + *mub * p43.z;
   */

   algebra::Vector3D ra;
   if (fa < 1 && fa > 0) {
     ra = sa.first + fa *va;
   } else if (fa <=0) {
     ra= sa.first;
   } else {
     ra= sa.second;
   }
   algebra::Vector3D rb;
   if (fb < 1 && fb > 0) {
     rb = sb.first + fb *vb;
   } else if (fa <=0) {
     rb= sb.first;
   } else {
     rb= sb.second;
   }

   IMP_LOG(VERBOSE, fa << " " << fb << std::endl);

   return Segment(ra, rb);
}


BondBondPairScore
::BondBondPairScore(UnaryFunction *f,
                    FloatKey volume): f_(f),
                                      volume_(volume)
{
}

Segment BondBondPairScore::get_endpoints(Particle *p) const
{
  atom::BondDecorator b(p);
  Particle* p0= b.get_bonded(0).get_particle();
  Particle* p1= b.get_bonded(1).get_particle();
  XYZDecorator d0(p0);
  XYZDecorator d1(p1);
  return Segment(d0.get_coordinates(), d1.get_coordinates());
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

algebra::Vector3D BondBondPairScore::get_difference(const Segment &s0,
                                                    const Segment &s1) const
{
  Segment s= shortest_segment(s0, s1);
  return s.second-s.first;
}


Float BondBondPairScore::evaluate(Particle *a, Particle *b,
                                          DerivativeAccumulator *da) const
{

  Segment ep[2]= {get_endpoints(a), get_endpoints(b)};
  Float d[2];
  for (int i=0; i< 2; ++i) {
    d[i]= (ep[i].first-ep[i].second).get_magnitude();
  }

  Float o[2]= {get_offset(a, d[0]),
               get_offset(b, d[1])};
  algebra::Vector3D diff= get_difference(ep[0], ep[1]);

  algebra::Vector3D deriv;
  Float score= internal::compute_distance_pair_score(diff, f_,
                                                     (da? &deriv : NULL),
                                                     boost::lambda::_1
                                                     - o[0] - o[1]);
  if (da) {
    XYZDecorator xyzd[2]={XYZDecorator(a), XYZDecorator(b)};
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

#endif // IMP_NO_DEPRECATED

IMPCORE_END_NAMESPACE
