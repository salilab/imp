/**
 *  \file xyzr_geometry.h
 *  \brief Represent an XYZRDecorator particle with a sphere
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_XYZR_GEOMETRY_H
#define IMPDISPLAY_XYZR_GEOMETRY_H

#include "config.h"
#include "macros.h"

#include "internal/version_info.h"
#include <IMP/PairContainer.h>
#include <IMP/core/XYZRDecorator.h>
#include <IMP/display/geometry.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Represent an XYZRDecorator particle with a sphere
/** The name is the Particle::get_name() name.
 */
class IMPDISPLAYEXPORT XYZRGeometry: public Geometry
{
  core::XYZRDecorator d_;
  FloatKey rk_;
public:
  //! Get the individual particles from the passed SingletonContainer
  XYZRGeometry(core::XYZRDecorator d);

  virtual ~XYZRGeometry();

  virtual Float get_size() const;

  std::string get_name() const {
    return d_.get_particle()->get_name();
  }
  virtual unsigned int get_dimension() const;
  virtual algebra::Vector3D get_vertex(unsigned int i) const;
  virtual VersionInfo get_version_info() const
  {return internal::version_info;}
  virtual void show(std::ostream &out=std::cout) const;

  //IMP_GEOMETRY(internal::version_info)
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_XYZR_GEOMETRY_H */
