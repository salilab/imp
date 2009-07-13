/**
 *  \file bond_geometry.h
 *  \brief Represent an XYZR particle with a sphere
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_BOND_GEOMETRY_H
#define IMPDISPLAY_BOND_GEOMETRY_H

#include "config.h"
#include "macros.h"

#include "internal/version_info.h"
#include <IMP/PairContainer.h>
#include <IMP/atom/bond_decorators.h>
#include <IMP/display/geometry.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Represent a bond with segment
/** A bond is represented by a segment with a given, constant
    radius. The name is taken from the name of the bond particle.
 */
class IMPDISPLAYEXPORT BondGeometry: public Geometry
{
  atom::Bond d_;
  Float radius_;
public:
  BondGeometry(atom::Bond d, Float radius=0);

  Color get_color() const;

  IMP_GEOMETRY(BondGeometry, internal::version_info)
};

//! Represent a set of bonds with segments
/** \see BondGeometry
 */
class IMPDISPLAYEXPORT BondsGeometry: public CompoundGeometry
{
  Pointer<SingletonContainer> sc_;
  double r_;
  FloatKey rk_;
public:
  BondsGeometry(SingletonContainer *sc, FloatKey radiuskey);

  BondsGeometry(SingletonContainer *sc, double r);

  IMP_COMPOUND_GEOMETRY(BondsGeometry, internal::version_info)
};

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_BOND_GEOMETRY_H */
