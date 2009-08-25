/**
 *  \file BoxGeometry.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_BOX_GEOMETRY_H
#define IMPDISPLAY_BOX_GEOMETRY_H

#include "config.h"
#include "macros.h"
#include "geometry.h"
#include <IMP/algebra/BoundingBoxD.h>
#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
IMPDISPLAY_BEGIN_NAMESPACE

//! Display an axis aligned wire frame box
/** The box is axis aligned and defined by the min and max corners.
 */
class IMPDISPLAYEXPORT BoxGeometry: public CompoundGeometry
{
  algebra::BoundingBox3D bb_;
  Color color_;
public:
  BoxGeometry(const algebra::BoundingBox3D &bb,
              const Color&color=Color());
  BoxGeometry(const algebra::Vector3D &min,
              const algebra::Vector3D &max,
              const Color &color=Color());

  IMP_COMPOUND_GEOMETRY(BoxGeometry, get_module_version_info())
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_BOX_GEOMETRY_H */
