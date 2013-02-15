/**
 *  \file  Segment3D.cpp
 *  \brief simple implementation of segments in 3D
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP/algebra/shortest_segment.h>
#include <IMP/base/log.h>


IMPALGEBRA_BEGIN_NAMESPACE


namespace {
  Vector3D get_point(const Segment3D &s, double f) {
    return s.get_point(0)+ f*(s.get_point(1)-s.get_point(0));
  }
  Segment3D get_reversed(const Segment3D &s) {
    return Segment3D(s.get_point(1), s.get_point(0));
  }
  Vector3D get_clipped_point(const Segment3D &s, double f) {
    if (f<= 0) return s.get_point(0);
    else if (f>=1) return s.get_point(1);
    else {
      return get_point(s,f);
    }
  }
  Segment3D get_shortest_segment_parallel(const Segment3D &sa,
                                          const Segment3D &sb) {
    // see if the endpoints of sb overlap sa
    for (int ep= 0; ep<2; ++ep) {
      double f= get_relative_projection_on_segment
        (sa, sb.get_point(ep));
      if (f >=0 && f <=1) {
        return Segment3D(get_point(sa, f), sb.get_point(ep));
      }
    }
    // see if the endpoints of sa overlap sb
    for (int ep= 0; ep<2; ++ep) {
      double f= get_relative_projection_on_segment(sb, sa.get_point(ep));
      if (f >=0 && f <=1) {
        return Segment3D(sa.get_point(ep), get_point(sb, f));
      }
    }
    // no overlap
    Segment3D ms(Vector3D(0,0,0), Vector3D(0,0,0));
    double md2=std::numeric_limits<double>::infinity();
    for (int i=0; i< 2; ++i) {
      for (int j=0; j< 2; ++j) {
        Segment3D c= Segment3D(sa.get_point(i), sb.get_point(j));
        double cm=(c.get_point(1)-c.get_point(0)).get_squared_magnitude();
        if (cm < md2) {
          ms=c;
          md2=cm;
        }
      }
    }
    return ms;
  }
}

Segment3D get_shortest_segment(const Segment3D &s,
                               const algebra::Vector3D &p) {
  double f= get_relative_projection_on_segment(s, p);
  return Segment3D(get_clipped_point(s, f), p);
}

/*
  Calculate the line segment PaPb that is the shortest route between
  two lines P1P2 and P3P4. Calculate also the values of mua and mub where
  Pa = P1 + mua (P2 - P1)
  Pb = P3 + mub (P4 - P3)
  Return FALSE if no solution exists.
*/
Segment3D get_shortest_segment(const Segment3D &sa,
                               const Segment3D &sb) {
  const double eps= .0001;
  algebra::Vector3D va= sa.get_point(1) - sa.get_point(0);
  algebra::Vector3D vb= sb.get_point(1) - sb.get_point(0);
  double ma= va*va;
  double mb= vb*vb;
  // if one of them is too short to have a well defined direction
  // just look at an endpoint
  if (ma < eps && mb < eps) {
    return Segment3D(sa.get_point(0), sb.get_point(0));
  } else if (ma < eps) {
    return get_reversed(get_shortest_segment(sb, sa.get_point(0)));
  } else if (mb < eps) {
    return get_shortest_segment(sa, sb.get_point(0));
  }

  algebra::Vector3D vfirst = sa.get_point(0)- sb.get_point(0);

  IMP_LOG_VERBOSE( vfirst << " | " << va << " | " << vb << std::endl);

  double dab = vb*va;

  double denom = ma * mb - dab * dab;
  // they are parallel or anti-parallel
  if (std::abs(denom) < eps) {
    return get_shortest_segment_parallel(sa, sb);
  }
  double dfb = vfirst*vb;
  double dfa = vfirst*va;
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

  algebra::Vector3D ra=get_clipped_point(sa, fa);
  algebra::Vector3D rb=get_clipped_point(sb, fb);

  IMP_LOG_VERBOSE( fa << " " << fb << std::endl);

  return Segment3D(ra, rb);
}

IMPALGEBRA_END_NAMESPACE
