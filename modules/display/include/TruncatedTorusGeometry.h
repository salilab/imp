/**
 *  \file IMP/display/TruncatedTorusGeometry.h
 *  \brief A geometry for a slice of a torus, 
 *         capped at the ends by half-spheres
 *
 *  \see IMP::core::TruncatedTorus
 *  \see IMP::core::BoundingTruncatedTorusSingletonScore
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_TRUNCATED_TORUS_GEOMETRY_H
#define IMPDISPLAY_TRUNCATED_TORUS_GEOMETRY_H

#include "display_config.h"
#include <IMP/core/TruncatedTorus.h
#include <IMP/display/geometry.h>
#include <IMP/display/display_macros.h>

IMPDISPLAY_BEGIN_NAMESPACE

/** A geometry for a slice of a torus, capped at the ends
    by half spheres. The truncated torus lies parallel to the XY-plane,
    and its central axis follows an arc starting at (R, 0, 0) and ending
    after $\Theta$ radians, going counterclockwise.
 */
class IMPDISPLAYEXPORT TruncatedTorusWireGeometry
: public Geometry {

  double R_; // major radius (of central axis of torus tube)
  double r_; // minor radius (about central axis of torus tube)
  double theta_; // length of torus arc in angle

 public:
  /** 
      A truncated torus with specified parameters.
  
      @param R major radius (radius of the central axis of the torus tube)
      @param r minor radius (radius of torus tube about its central axis)
      @param theta the length of the torus arc in radians
  */
  TruncatedTorusWireGeometry(double R, double r, double theta);

  //! A truncated torus with parameters retrieved from decorator truncated_torus 
  TruncatedTorusWireGeometry(const TruncatedTorus& truncated_torus);

  //! returns the set of geometric components that comprise this geometry
  //! (for e.g. storing in RMF format)
  virtual IMP::display::Geometries get_components() const IMP_OVERRIDE;

  IMP_OBJECT_METHODS(TruncatedTorusWireGeometry);
};


IMPDISPLAY_END_NAMESPACE

#endif /* IMPDISPLAY_TRUNCATED_TORUS_GEOMETRY_H */
