/**
 *  \file CoverBondSingletonModifier.h
 *  \brief Cover a bond with a sphere.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_COVER_BOND_SINGLETON_MODIFIER_H
#define IMPCORE_COVER_BOND_SINGLETON_MODIFIER_H

#include "core_exports.h"
#include "internal/core_version_info.h"

#include "SingletonModifier.h"
#include "SingletonContainer.h"
#include "XYZRDecorator.h"
#include <IMP/Pointer.h>

IMPCORE_BEGIN_NAMESPACE

//! This class sets the position and radius of each bond to cover the endpoints.
/** Set the coordinates and radius of the passed particle to cover the bond.
    Note that the particle passed must be both a BondDecorator and an
    XYZRDecorator.

    Note that the ends of the bond are treated as points so any radius
    they might have is ignored. This would be easy to change if
    desired.

    \ingroup bond
 */
class IMPCOREEXPORT CoverBondSingletonModifier: public SingletonModifier
{
  FloatKey rk_;
  Float slack_;
public:
  CoverBondSingletonModifier(FloatKey rk
                            =XYZRDecorator::get_default_radius_key(),
                            Float slack=0);
  ~CoverBondSingletonModifier();

  IMP_SINGLETON_MODIFIER(internal::core_version_info);

  void set_slack(Float slack) {
    slack_=slack;
  }
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_COVER_BOND_SINGLETON_MODIFIER_H */
