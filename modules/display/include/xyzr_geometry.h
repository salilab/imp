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

#include "internal/version_info.h"
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

  std::string get_name() const {
    return d_.get_particle()->get_name();
  }
  IMP_GEOMETRY(XYZRGeometry, internal::version_info)
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_XYZR_GEOMETRY_H */
