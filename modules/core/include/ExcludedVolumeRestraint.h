/**
 *  \file core/ExcludedVolumeRestraint.h
 *  \brief A prevent spheres from inter-penetrating
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_EXCLUDED_VOLUME_RESTRAINT_H
#define IMPCORE_EXCLUDED_VOLUME_RESTRAINT_H

#include "config.h"

#include "internal/version_info.h"
#include "internal/remove_pointers.h"
#include "PairsRestraint.h"
#include "ClosePairsScoreState.h"
#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Restraint.h>
#include <IMP/UnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE

//! Prevent a set of particles and rigid bodies from inter-penetrating
/** Given an arbitrary collection of particles and rigid bodies, this
    restraint prevents the particles from interpenetrating.

    If any of the rigid bodies don't have radii, an appropriate radius
    will be added. Note, to take advantage of this, the RigidBody
    must be in the container before the Restraint is added to the model.
 */
class IMPCOREEXPORT ExcludedVolumeRestraint: public Restraint
{
  Pointer<ClosePairsScoreState> ss_;
  Pointer<SingletonContainer> sc_;
  Pointer<PairsRestraint> pr_;
  double k_;
public:
  /** The SingletonContainer contains a set of XYZR particles and RigidBody
      particles. The spring constant used is k.*/
  ExcludedVolumeRestraint(SingletonContainer *sc,
                          double k=1);

  IMP_RESTRAINT(ExcludedVolumeRestraint, internal::version_info);

  virtual ParticlesList get_interacting_particles() const;

  void set_model(Model *m);
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_EXCLUDED_VOLUME_RESTRAINT_H */
