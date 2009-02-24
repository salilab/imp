/**
 *  \file CylinderGeometry.h
 *  \brief Represent an CylinderDecorator particle with a sphere
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_CYLINDER_GEOMETRY_H
#define IMPDISPLAY_CYLINDER_GEOMETRY_H

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
class IMPDISPLAYEXPORT CylinderGeometry: public Geometry
{
  algebra::Vector3D p_[2];
  double radius_;
public:
  //! Create a static cylinder or segment
  /** If the radius is skipped, it is a segment. If the color is skipped,
      the default color (gray) is used.
  */
  CylinderGeometry(const algebra::Vector3D& p0,
                   const algebra::Vector3D& p1,
                   double radius=0);

  virtual ~CylinderGeometry();

  virtual Float get_size() const {
    return radius_;
  }


  virtual unsigned int get_dimension() const;
  virtual algebra::Vector3D get_vertex(unsigned int i) const;
  virtual VersionInfo get_version_info() const
  {return internal::version_info;}
  virtual void show(std::ostream &out=std::cout) const;

  //IMP_GEOMETRY(internal::version_info)
};

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_CYLINDER_GEOMETRY_H */
