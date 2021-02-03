/**
 *  \file IMP/display/restraint_geometry.h
 *  \brief Represent an XYZR restraint with a sphere
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_RESTRAINT_GEOMETRY_H
#define IMPDISPLAY_RESTRAINT_GEOMETRY_H

#include <IMP/display/display_config.h>
#include "geometry_macros.h"
#include "Colored.h"
#include "declare_Geometry.h"
#include <IMP/Pointer.h>
#include <IMP/PairContainer.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Try to draw some stuff for a generic restraint
/** Mostly it just breaks the restraint down as much as possible and shows
    the particles involved in the constituent terms if they are XYZ particles.

    Note, make sure the model is up to date before calling this, for reasons of
    efficiency, it does not update the model.
*/
class IMPDISPLAYEXPORT RestraintGeometry : public Geometry {
  IMP::PointerMember<Restraint> r_;
  IMP::Pointer<Model> m_;
  //! Use this if the restraint is not part of the model
  RestraintGeometry(Restraint *p, Model *m);

 public:
  RestraintGeometry(Restraint *p);
  Restraint *get_restraint() const { return r_; }
  virtual IMP::display::Geometries get_components() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RestraintGeometry);
};

//! Geometry for a whole set of restraints
class IMPDISPLAYEXPORT RestraintSetGeometry : public Geometry {
  IMP::Pointer<RestraintSet> r_;

 public:
  RestraintSetGeometry(RestraintSet *p);
  RestraintSet *get_restraint_set() const { return r_; }
  virtual IMP::display::Geometries get_components() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RestraintSetGeometry);
};

/** @} */
IMPDISPLAY_END_NAMESPACE

#endif /* IMPDISPLAY_RESTRAINT_GEOMETRY_H */
