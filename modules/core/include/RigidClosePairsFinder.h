/**
 *  \file IMP/core/RigidClosePairsFinder.h
 *  \brief Handle rigid bodies by looking at their members
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_RIGID_CLOSE_PAIRS_FINDER_H
#define IMPCORE_RIGID_CLOSE_PAIRS_FINDER_H

#include "ClosePairsFinder.h"
#include "rigid_bodies.h"
#include "close_pairs_finder_macros.h"
#include <IMP/kernel/internal/InternalListSingletonContainer.h>
#include <IMP/Refiner.h>

IMPCORE_BEGIN_NAMESPACE

//! Peform more efficient close pair finding when rigid bodies are involved.
/** The class finds all close pairs consisting of particles taken from the
    passed list(s) (if they are not rigid bodies) or members of rigid
    bodies passed as input. That is, given an input list \c l, for each
    pair of particles \c p, \c q taken from the list, that are closer than
    the distance threshold, it returns
    - if neither \c p or \c q are RigidBody particles it returns
    (\c p,\c q)
    - if bother \c p and \c q are RigidBody particles, it returns
    all \c (\c r,\c s) where \c r is a member of \c p and \c s is member of
    \c q and \c r and \c s are closer than the distance threshold
    - pairs \c (\c p,\c s) or \c (\c r,\c q) as appropriate if only one of
    \c p or \c q is a rigid body.

    Consequently, the user must ensure that the RigidBody are
    assigned a radius that encloses all of their RigidMember
    particles.

    It uses another ClosePairsFinder to find which pairs of particles in
    the input list or lists are close. Your choice of this can be passed
    to the constructor.

    \note The bipartite method will also not return any pairs where
    both members are in the same rigid body.

    \note The bounding spheres are kept in internal coordinates for
    the rigid body and transformed on the fly. It would probably be
    faster to cache the tranformed results.

    \note The particles are divided up using a grid. The number of
    grid cells to use should be explored. In addition, with highly
    excentric sets of points, there will be too many cells.

    \note Do not reuse RigidClosePairsFinders for different sets of
    particles from the same rigid body.

    \include rigid_collisions.py

    \uses{class RigidClosePairsFinder, CGAL}
    \see ClosePairsScoreState
    \see RigidBody
    \see cover_members()
 */
class IMPCOREEXPORT RigidClosePairsFinder : public ClosePairsFinder {
  mutable IMP::base::PointerMember<ClosePairsFinder> cpf_;
  ObjectKey k_;

 public:
  RigidClosePairsFinder(ClosePairsFinder *cpf = nullptr);

  kernel::ParticleIndexPairs get_close_pairs(
      kernel::Model *m, kernel::ParticleIndex a, kernel::ParticleIndex b,
      const kernel::ParticleIndexes &pa,
      const kernel::ParticleIndexes &pb) const;

  void set_distance(double d) {
    cpf_->set_distance(d);
    ClosePairsFinder::set_distance(d);
  }

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  internal::MovedSingletonContainer *get_moved_singleton_container(
      SingletonContainer *c, double thresold) const;
#endif
#ifndef SWIG
  using ClosePairsFinder::get_close_pairs;
#else
  kernel::ParticlePairsTemp get_close_pairs(const kernel::ParticlesTemp &pc)
      const;
  kernel::ParticlePairsTemp get_close_pairs(
      const kernel::ParticlesTemp &pca, const kernel::ParticlesTemp &pcb) const;
#endif

  virtual IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bbs) const
      IMP_OVERRIDE;
  virtual IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bas,
                                   const algebra::BoundingBox3Ds &bbs) const
      IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;

  virtual kernel::ParticleIndexPairs get_close_pairs(
      kernel::Model *m, const kernel::ParticleIndexes &pc) const IMP_OVERRIDE;
  virtual kernel::ParticleIndexPairs get_close_pairs(
      kernel::Model *m, const kernel::ParticleIndexes &pca,
      const kernel::ParticleIndexes &pcb) const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RigidClosePairsFinder);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_RIGID_CLOSE_PAIRS_FINDER_H */
