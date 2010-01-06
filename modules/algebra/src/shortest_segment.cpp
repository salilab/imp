/**
 *  \file  Segment3D.cpp
 *  \brief simple implementation of segments in 3D
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */
#include <IMP/algebra/shortest_segment.h>
#include <IMP/log.h>


IMPALGEBRA_BEGIN_NAMESPACE

Segment3D shortest_segment(const Segment3D &s, const algebra::Vector3D &p) {
  algebra::Vector3D vs= s.get_point(1)- s.get_point(0);
  algebra::Vector3D vps= p- s.get_point(0);
  double f= vps*vs/(vs*vs);
  if (f<= 0) return Segment3D(s.get_point(0), p);
  else if (f>=1) return Segment3D(s.get_point(1), p);
  else {
    algebra::Vector3D ps= s.get_point(0) + vs*f;
    return Segment3D(ps, p);
  }
}

/*
  Calculate the line segment PaPb that is the shortest route between
  two lines P1P2 and P3P4. Calculate also the values of mua and mub where
  Pa = P1 + mua (P2 - P1)
  Pb = P3 + mub (P4 - P3)
  Return FALSE if no solution exists.
*/
Segment3D shortest_segment(const Segment3D &sa,
                           const Segment3D &sb) {
  const double eps= .0001;
  algebra::Vector3D va= sa.get_point(1) - sa.get_point(0);
  algebra::Vector3D vb= sb.get_point(1) - sb.get_point(0);
  double ma= va*va;
  double mb= vb*vb;
  if (ma < eps) {
    if (mb < eps) {
      return Segment3D(sa.get_point(0), sb.get_point(0));
    } else {
      Segment3D sr =shortest_segment(sb, sa.get_point(0));
      return Segment3D(sr.get_point(1), sr.get_point(0));
    }
  } else if (mb < eps) {
    return shortest_segment(sa, sb.get_point(0));
  }

  algebra::Vector3D vfirst = sa.get_point(0)- sb.get_point(0);

  IMP_LOG(VERBOSE, vfirst << " | " << va << " | " << vb << std::endl);

  double dfb = vfirst*vb;
  double dab = vb*va;
  double dfa = vfirst*va;

  double denom = ma * mb - dab * dab;
  if (std::abs(denom) < eps) {
    return Segment3D(sa.get_point(0), sb.get_point(0));
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
    ra = sa.get_point(0) + fa *va;
  } else if (fa <=0) {
    ra= sa.get_point(0);
  } else {
    ra= sa.get_point(1);
  }
  algebra::Vector3D rb;
  if (fb < 1 && fb > 0) {
    rb = sb.get_point(0) + fb *vb;
  } else if (fa <=0) {
    rb= sb.get_point(0);
  } else {
    rb= sb.get_point(1);
  }

  IMP_LOG(VERBOSE, fa << " " << fb << std::endl);

  return Segment3D(ra, rb);
}

IMPALGEBRA_END_NAMESPACE
