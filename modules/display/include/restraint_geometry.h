/**
 *  \file restraint_geometry.h
 *  \brief Represent an XYZR restraint with a sphere
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
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

//! Draw edges for various core::PairRestraint.
/** This currently can handle
    - IMP::core::PairRestraint

*/
class IMPDISPLAYEXPORT PairRestraintGeometry: public Geometry
{
  IMP::Pointer<core::PairRestraint> r_;
public:
  PairRestraintGeometry(core::PairRestraint *p);
  core::PairRestraint *get_restraint() const {
    return r_;
  }
  IMP_GEOMETRY(PairRestraintGeometry);
};

//! Draw edges for various container::PasirRestraint.
/** This currently can handle
    - IMP::container::PairsRestraint

*/
class IMPDISPLAYEXPORT PairsRestraintGeometry: public Geometry
{
  IMP::Pointer<container::PairsRestraint> r_;
public:
  PairsRestraintGeometry(container::PairsRestraint *p);
  container::PairsRestraint *get_restraint() const {
    return r_;
  }
  IMP_GEOMETRY(PairsRestraintGeometry);
};

//! Try to draw some stuff for a generic restraint
class IMPDISPLAYEXPORT RestraintGeometry: public Geometry {
  IMP::Pointer<Restraint> r_;
public:
  RestraintGeometry(Restraint *p);
  Restraint *get_restraint() const {
    return r_;
  }
  IMP_GEOMETRY(RestraintGeometry);
};

//! Draw edges for core::ConnectivityRestraint.
/** It special cases IMP::core::KClosePairsPairScore.

*/
class IMPDISPLAYEXPORT ConnectivityRestraintGeometry: public Geometry
{
  IMP::Pointer<core::ConnectivityRestraint> r_;
public:
  ConnectivityRestraintGeometry(core::ConnectivityRestraint *p);
  Restraint *get_restraint() const {
    return r_;
  }
  IMP_GEOMETRY(ConnectivityRestraintGeometry);
};


/** Attempt to create restraint geometry for the passed restraint.
    \throws ValueException if it can't handle the restraint.
 */
IMPDISPLAYEXPORT Geometry* create_restraint_geometry(Restraint *r);

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_RESTRAINT_GEOMETRY_H */
