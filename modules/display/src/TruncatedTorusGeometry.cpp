/**
 *  \file TruncatedTorusGeometry.cpp
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/TruncatedTorusGeometry.h"
#include <IMP/display/geometry.h>
#include <IMP/constants.h>
#include <IMP/algebra/Plane3D.h>
#ifdef IMP_DISPLAY_USE_IMP_CGAL
#include <IMP/cgal/internal/polyhedrons.h>
#endif
#include <boost/tuple/tuple.hpp>
#include <algorithm>
#include <cmath>
#include <set>

IMPDISPLAY_BEGIN_NAMESPACE

#define N_SEGMENTS_CIRCLE 30

TruncatedTorusWireGeometry::TruncatedTorusWireGeometry
(double R, double r, double theta)
  : Geometry("TruncatedTorus"),
    R_(R), r_(r), theta_(theta)
{}

display::Geometries
TruncatedTorusWireGeometry::get_components() const
{
  display::Geometries ret;
  // number of segmennts is proportional to theta_,
  // the size of the truncated torus arc in radians
  const int n1= N_SEGMENTS_CIRCLE * round(theta_ / (2 * IMP::PI)) + 1; 
  const int n2= N_SEGMENTS_CIRCLE;
  for (int i= 1; i <= n1; ++i) { // major radius
    double f= static_cast<double>(i) / n1;
    double f1= static_cast<double>(i - 1) / n1;
    double gamma= theta_ * f; // make segments from 0 to theta_ radians
    double gamma1= theta_ * f1;
    algebra::Vector3D v00(R_ * sin(gamma),
                          R_ * cos(gamma),
                          r_);
    for (int j= 0; j <= n2; ++j) { // minor radius
      double g= static_cast<double>(j) / n2;
      double omega= 2 * IMP::PI * g;
      double dZ= r_ * cos(omega);
      double dXY= R_ - r_ * sin(omega);
      algebra::Vector3D v10(dXY * sin(gamma),
                           dXY * cos(gamma),
                           dZ);
      algebra::Vector3D v11(dXY * sin(gamma1),
                           dXY * cos(gamma1),
                           dZ);
      ret.push_back(new display::SegmentGeometry(algebra::Segment3D(v00, v10))); // along major radius
      ret.push_back(new display::SegmentGeometry(algebra::Segment3D(v10, v11))); // along minor radius (tube)
      v00=v10;
    } // j
  }  // i
  // TODO: add caps
  return ret;
}

IMPDISPLAY_END_NAMESPACE
