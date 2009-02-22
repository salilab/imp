/**
 *  \file SphereGeometry.h
 *  \brief Represent an SphereDecorator particle with a sphere
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_SPHERE_GEOMETRY_H
#define IMPDISPLAY_SPHERE_GEOMETRY_H

#include "config.h"
#include "macros.h"

#include "internal/version_info.h"
#include "geometry.h"
#include <IMP/PairContainer.h>
#include <IMP/algebra/Vector3D.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Represent a static sphere (or point)
/**
 */
class IMPDISPLAYEXPORT SphereGeometry: public Geometry
{
  algebra::Vector3D center_;
  double radius_;
  algebra::Vector3D color_;
  std::string name_;
public:
  //! Create a static sphere or point
  /** If the radius is skipped, it is a point. If the color is skipped,
      the default color (gray) is used.
  */
  SphereGeometry(algebra::Vector3D center,
                 double radius=0,
                 algebra::Vector3D color
                 = algebra::Vector3D(-1, -1, -1),
                 std::string name= std::string());

  virtual ~SphereGeometry();

  virtual Float get_size() const {
    return radius_;
  }

  std::string get_name() const {
    return name_;
  }

  virtual unsigned int get_dimension() const;
  virtual algebra::Vector3D get_vertex(unsigned int i) const;
  virtual VersionInfo get_version_info() const
  {return internal::version_info;}
  virtual void show(std::ostream &out=std::cout) const;

  //IMP_GEOMETRY(internal::version_info)
};

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_SPHERE_GEOMETRY_H */
