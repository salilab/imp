/**
 *  \file xyzr_geometry.h
 *  \brief Represent an XYZR particle with a sphere
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_XYZR_GEOMETRY_H
#define IMPDISPLAY_XYZR_GEOMETRY_H

#include "config.h"
#include "macros.h"

#include <IMP/PairContainer.h>
#include <IMP/core/XYZR.h>
#include <IMP/display/geometry.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Represent an XYZR particle with a sphere
/** The current coordinates and radius of the particle are used.
    The name is the Particle::get_name() name.
 */
class IMPDISPLAYEXPORT XYZRGeometry: public Geometry
{
  core::XYZR d_;
  FloatKey rk_;
public:
  XYZRGeometry(core::XYZR d);

  Color get_color() const;

  IMP_GEOMETRY(XYZRGeometry, get_module_version_info())
};

class IMPDISPLAYEXPORT XYZRsGeometry: public CompoundGeometry {
  Pointer<SingletonContainer> sc_;
  FloatKey rk_;
 public:
  XYZRsGeometry(SingletonContainer *sc,
                FloatKey rk=core::XYZR::get_default_radius_key());
  IMP_COMPOUND_GEOMETRY(XYZRsGeometry, get_module_version_info())
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_XYZR_GEOMETRY_H */
