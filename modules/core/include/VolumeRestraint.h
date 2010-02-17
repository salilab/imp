/**
 *  \file core/VolumeRestraint.h
 *  \brief A prevent spheres from inter-penetrating
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_VOLUME_RESTRAINT_H
#define IMPCORE_VOLUME_RESTRAINT_H

#include "config.h"

#include <IMP/algebra/Grid3D.h>

#include <IMP/SingletonContainer.h>
#include <IMP/Restraint.h>
#include <IMP/UnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE

//! Restraint the volume of a set of spheres
/** \untested{VolumeRestraint}*/
class IMPCOREEXPORT VolumeRestraint: public Restraint
{
  IMP::internal::OwnerPointer<SingletonContainer> sc_;
  IMP::internal::OwnerPointer<UnaryFunction> f_;
  double volume_;
  typedef algebra::Grid3D<int> Grid;
  mutable Grid grid_;
public:
  VolumeRestraint(UnaryFunction *f,
                  SingletonContainer *sc,
                  double volume);

  IMP_RESTRAINT(VolumeRestraint);
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_VOLUME_RESTRAINT_H */
