/**
 *  \file core/ExcludedVolumeRestraint.h
 *  \brief A prevent spheres from inter-penetrating
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_EXCLUDED_VOLUME_RESTRAINT_H
#define IMPCORE_EXCLUDED_VOLUME_RESTRAINT_H

#include "core_config.h"

#include "internal/remove_pointers.h"
#include "internal/CorePairsRestraint.h"
#include "internal/CoreClosePairContainer.h"
#include "rigid_bodies.h"
#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Restraint.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Refiner.h>

IMPCORE_BEGIN_NAMESPACE

//! Prevent a set of particles and rigid bodies from inter-penetrating
/** Given an arbitrary collection of particles and rigid bodies, this
    restraint prevents the particles from interpenetrating. Such restraints
    are also known as steric clash restraints.

    If any of the rigid bodies don't have radii, an appropriate radius
    will be added. Note, to take advantage of this, the RigidBody
    must be in the container before the Restraint is added to the model.
 */
class IMPCOREEXPORT ExcludedVolumeRestraint:
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  public internal::CorePairsRestraint
#else
  public Restraint
#endif
{
  Pointer<SingletonContainer> sc_;
public:
#ifndef IMP_DOXYGEN
  /** The SingletonContainer contains a set of XYZR particles and RigidBody
      particles. The spring constant used is k.*/
  ExcludedVolumeRestraint(SingletonContainer *sc,
                          Refiner *r,
                          double k=1);
#endif

  /** The SingletonContainer contains a set of XYZR particles and RigidMembers.

      The spring constant used is k.*/
  ExcludedVolumeRestraint(SingletonContainer *sc,
                          double k=1);

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  double unprotected_evaluate(DerivativeAccumulator *) const;
#endif

  void set_log_level(LogLevel l);

#ifdef SWIG
  IMP_RESTRAINT(ExcludedVolumeRestraint);
#endif
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_EXCLUDED_VOLUME_RESTRAINT_H */
