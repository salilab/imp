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

IMPDISPLAY_BEGIN_NAMESPACE

//! Draw edges for various pair-based restraints.
/** This currently can handle
    - IMP::container::PairsRestraint
    - IMP::core::PairRestraint

*/
class IMPDISPLAYEXPORT PairRestraintGeometry: public Geometry
{
  IMP::internal::OwnerPointer<PairContainer> pc_;
  IMP::Pointer<Restraint> r_;
public:
  PairRestraintGeometry(Restraint *p);
  Restraint *get_restraint() const {
    return r_;
  }
  IMP_GEOMETRY(PairRestraintGeometry);
};

//! Draw edges for various pair-based restraints.
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


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_RESTRAINT_GEOMETRY_H */
