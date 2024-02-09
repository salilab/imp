/**
 *  \file SlabWithToroidalPoreGeometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/npc/SlabWithToroidalPoreGeometry.h"
#include <IMP/display/geometry.h>

#include <IMP/constants.h>
#include <IMP/algebra/Plane3D.h>
#ifdef IMP_NPC_USE_IMP_CGAL
#include <IMP/cgal/internal/polyhedrons.h>
#endif
#include <boost/tuple/tuple.hpp>
#include <algorithm>
#include <set>

IMPNPC_BEGIN_NAMESPACE

//! Slab with specified height from top to bottom, slab_length x slab_length area,
//! and an elliptic toroidal pore of specified major radius,
//! slab_height/2.0 vertical semi-axis, specified horizontal semi-axis
SlabWithToroidalPoreWireGeometry
::SlabWithToroidalPoreWireGeometry
(double slab_height,
 double major_radius,
 double horizontal_semiaxis,
 double slab_length)
  : Geometry("SlabWithToroidalPore"),
    rv_(slab_height/2.0),
    rh_(horizontal_semiaxis),
    R_(major_radius),
    slab_length_(slab_length)
{}

//
SlabWithToroidalPoreWireGeometry
::SlabWithToroidalPoreWireGeometry
(double slab_height,
 double major_radius,
 double slab_length)
  : Geometry("SlabWithToroidalPore"),
    rv_(slab_height/2.0),
    rh_(slab_height/2.0),
    R_(major_radius),
    slab_length_(slab_length)
{}

//
display::Geometries
SlabWithToroidalPoreWireGeometry
::get_components() const
{
  display::Geometries ret;
  // Add trimmed toroidal pore:
  const int n1= 30;
  const int n2= 10;
  for (int i= 1; i <= n1; ++i) { // major radius
    double f= static_cast<double>(i) / n1;
    double f1= static_cast<double>(i - 1) / n1;
    double theta= 2 * IMP::PI * f;
    double theta1= 2 * IMP::PI * f1;
    algebra::Vector3D v00(R_ * sin(theta),
                          R_ * cos(theta),
                          rv_);
    for (int j= 0; j <= n2; ++j) { // minor radius
      double g= static_cast<double>(j) / n2;
      double omega= IMP::PI * g;
      double dZ= rv_ * cos(omega);
      double dXY= R_ - rh_ * sin(omega);
      algebra::Vector3D v10(dXY * sin(theta),
                           dXY * cos(theta),
                           dZ);
      algebra::Vector3D v11(dXY * sin(theta1),
                           dXY * cos(theta1),
                           dZ);
      ret.push_back(new display::SegmentGeometry(algebra::Segment3D(v00, v10))); // cylinder curved face
      ret.push_back(new display::SegmentGeometry(algebra::Segment3D(v10, v11))); // cylinder edge between theta and theta 1 from top to bottom
      v00=v10;
      if(j==0 || j == n2) {
        // Ray to slab edge:
        double isin= 1.0 / (sin(theta)+0.0000001);
        double icos= 1.0 / (cos(theta)+0.0000001);
        double d= 0.5 * slab_length_ * std::min(std::abs(isin), std::abs(icos));
        algebra::Vector3D v2(d * sin(theta),
                             d * cos(theta),
                             dZ);
        ret.push_back(new display::SegmentGeometry(algebra::Segment3D(v10, v2)));
      }
    } // j
  }  // i
  // Add top and bottom slab rectangles:
  for(int sign=-1; sign<=1; sign+=2){
    algebra::Vector3D vNE(0.5*slab_length_, 0.5*slab_length_, sign*rv_);
    algebra::Vector3D vNW(-0.5*slab_length_, 0.5*slab_length_, sign*rv_);
    algebra::Vector3D vSW(-0.5*slab_length_, -0.5*slab_length_, sign*rv_);
    algebra::Vector3D vSE(0.5*slab_length_, -0.5*slab_length_, sign*rv_);
    ret.push_back(new display::SegmentGeometry(algebra::Segment3D(vNE, vNW))); // slab bottom face
    ret.push_back(new display::SegmentGeometry(algebra::Segment3D(vNW, vSW))); // slab bottom face
    ret.push_back(new display::SegmentGeometry(algebra::Segment3D(vSW, vSE))); // slab bottom face
    ret.push_back(new display::SegmentGeometry(algebra::Segment3D(vSE, vNE))); // slab bottom face
  }
  return ret;
}

IMPNPC_END_NAMESPACE
