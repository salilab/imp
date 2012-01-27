/**
 *  \file restraint_geometry.h
 *  \brief Represent an XYZR restraint with a sphere
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_RESTRAINT_GEOMETRY_H
#define IMPDISPLAY_RESTRAINT_GEOMETRY_H

#include "display_config.h"
#include "display_macros.h"
#include "Colored.h"
#include "geometry.h"
#include <IMP/core/ConnectivityRestraint.h>
#include <IMP/Pointer.h>
#include <IMP/PairContainer.h>
#include <IMP/core/PairRestraint.h>
#include <IMP/container/PairsRestraint.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Try to draw some stuff for a generic restraint
/** Mostly it just breaks the restraint down as much as possible and shows
    the particles involved in the constituent terms if they are XYZ particles.
*/
class IMPDISPLAYEXPORT RestraintGeometry: public Geometry {
  IMP::Pointer<Restraint> r_;
  IMP::Pointer<Model> m_;
  //! Use this if the restraint is not part of the model
  RestraintGeometry(Restraint *p, Model *m);
public:
  RestraintGeometry(Restraint *p);
  Restraint *get_restraint() const {
    return r_;
  }
  IMP_GEOMETRY(RestraintGeometry);
};


//! Geometry for a whole set of restraints
class IMPDISPLAYEXPORT RestraintSetGeometry: public Geometry {
  IMP::Pointer<RestraintSet> r_;
public:
  RestraintSetGeometry(RestraintSet *p);
  RestraintSet *get_restraint_set() const {
    return r_;
  }
  IMP_GEOMETRY(RestraintSetGeometry);
};



/** @} */
IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_RESTRAINT_GEOMETRY_H */
