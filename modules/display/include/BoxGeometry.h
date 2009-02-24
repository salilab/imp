/**
 *  \file BoxGeometry.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_BOX_GEOMETRY_H
#define IMPDISPLAY_BOX_GEOMETRY_H

#include "config.h"
#include "macros.h"
#include "geometry.h"
#include "internal/version_info.h"
#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
IMPDISPLAY_BEGIN_NAMESPACE

//! Display a bounding box
/** Defined by the min and max corners.
 */
class IMPDISPLAYEXPORT BoxGeometry: public CompoundGeometry
{
  algebra::Vector3D min_, max_;
  Color color_;
public:
  //! XXXX
  BoxGeometry(const algebra::Vector3D &min,
              const algebra::Vector3D &max,
              const Color &color=Color());

  virtual ~BoxGeometry();

  IMP_COMPOUND_GEOMETRY(internal::version_info)
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_BOX_GEOMETRY_H */
