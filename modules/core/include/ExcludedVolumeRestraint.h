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
#include "RigidClosePairsFinder.h"
#include <IMP/core/SphereDistancePairScore.h>

IMPCORE_BEGIN_NAMESPACE

//! Prevent a set of particles and rigid bodies from inter-penetrating
/** Given an arbitrary collection of particles and rigid bodies, this
    restraint prevents the particles from interpenetrating. Such restraints
    are also known as steric clash restraints.

    \note If any of the rigid bodies don't have radii, an appropriate radius
    will be added. Note, to take advantage of this, the RigidBody
    must be in the container before the Restraint is added to the model.

    \note Changing the set of particles in the SingletonContainer is not
    currently supported after the first evaluate call.

    \not Currently the radius of all particles is assumed to be constant
 */
class IMPCOREEXPORT ExcludedVolumeRestraint: public Restraint
{
  Pointer<SingletonContainer> sc_;
  mutable ParticleIndexPairs cur_list_;
  mutable bool was_bad_;
  mutable bool initialized_;
  ObjectKey key_;
  IMP::internal::OwnerPointer<SoftSpherePairScore> ssps_;
  // moved stuff
  mutable ParticleIndexes rbs_;
  mutable ParticleIndexes xyzrs_;
  mutable IMP::compatibility::map<ParticleIndex, ParticleIndexes> constituents_;
  double slack_;
  mutable std::vector<algebra::Transformation3D > rbs_backup_;
  mutable std::vector<algebra::Vector3D> xyzrs_backup_;

  void reset_moved() const;
  void initialize() const;
  int get_if_moved() const;
  void fill_list() const;
  double fill_list_if_good(double max) const;
public:
  /** The SingletonContainer contains a set of XYZR particles and RigidMembers.

      The spring constant used is k.*/
  ExcludedVolumeRestraint(SingletonContainer *sc,
                          double k=1, double slack_=10);

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  double unprotected_evaluate_if_good(DerivativeAccumulator *da,
                                      double max) const;
#endif
  IMP_RESTRAINT(ExcludedVolumeRestraint);
  Restraints create_decomposition() const;
  Restraints create_current_decomposition() const;
  IMP_LIST(public, PairFilter, pair_filter,
           PairFilter*, PairFilters);
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_EXCLUDED_VOLUME_RESTRAINT_H */
