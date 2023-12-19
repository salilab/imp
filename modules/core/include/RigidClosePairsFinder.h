/**
 *  \file IMP/core/RigidClosePairsFinder.h
 *  \brief Handle rigid bodies by looking at their members
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_RIGID_CLOSE_PAIRS_FINDER_H
#define IMPCORE_RIGID_CLOSE_PAIRS_FINDER_H

#include "ClosePairsFinder.h"
#include "rigid_bodies.h"
#include <IMP/Refiner.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPCORE_BEGIN_NAMESPACE

//! Perform more efficient close pair finding when rigid bodies are involved.
/** The class finds all close pairs consisting of particles taken from the
    passed list(s) (if they are not rigid bodies) or members of rigid
    bodies passed as input. That is, given an input list \c l, for each
    pair of particles \c p, \c q taken from the list that are closer than
    the distance threshold, it returns
    - (\c p,\c q) if neither \c p or \c q are RigidBody particles
    - all \c (\c r,\c s) where \c r is a member of \c p, \c s is a member of
    \c q, and \c r and \c s are closer than the distance threshold if both
    \c p and \c q are RigidBody particles
    - pairs \c (\c p,\c s) or \c (\c r,\c q) as appropriate if only one of
    \c p or \c q is a rigid body.

    Consequently, the user must ensure that each RigidBody is
    assigned a radius that encloses all of its RigidMember
    particles.

    It uses another ClosePairsFinder to find which pairs of particles in
    the input list or lists are close. Your choice of this can be passed
    to the constructor.

    \note The bipartite method will also not return any pairs where
    both members are in the same rigid body.

    \note The bounding spheres are kept in internal coordinates for
    the rigid body and transformed on the fly. It would probably be
    faster to cache the transformed results.

    \note The particles are divided up using a grid. The number of
    grid cells to use should be explored. In addition, with highly
    eccentric sets of points, there will be too many cells.

    \note Do not reuse RigidClosePairsFinders for different sets of
    particles from the same rigid body.

    \include rigid_collisions.py

    \uses{class RigidClosePairsFinder, CGAL}
    \see ClosePairsScoreState
    \see RigidBody
    \see cover_members()
 */
class IMPCOREEXPORT RigidClosePairsFinder : public ClosePairsFinder {
  mutable IMP::PointerMember<ClosePairsFinder> cpf_;
  ObjectKey k_;

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<ClosePairsFinder>(this), cpf_);
    if (std::is_base_of<cereal::detail::InputArchiveBase, Archive>::value) {
      k_ = get_hierarchy_key();
    }
  }
  IMP_OBJECT_SERIALIZE_DECL(RigidClosePairsFinder);

  ObjectKey get_hierarchy_key() const;

 public:
  RigidClosePairsFinder(ClosePairsFinder *cpf = nullptr);

  ParticleIndexPairs get_close_pairs(
      Model *m, ParticleIndex a, ParticleIndex b,
      const ParticleIndexes &pa,
      const ParticleIndexes &pb) const;

  void set_distance(double d) override {
    cpf_->set_distance(d);
    ClosePairsFinder::set_distance(d);
  }

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  internal::MovedSingletonContainer *get_moved_singleton_container(
      SingletonContainer *c, double threshold) const override;
#endif
  virtual IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bbs) const
      override;
  virtual IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bas,
                                   const algebra::BoundingBox3Ds &bbs) const
      override;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override;

  virtual ParticleIndexPairs get_close_pairs(
      Model *m, const ParticleIndexes &pc) const override;
  virtual ParticleIndexPairs get_close_pairs(
      Model *m, const ParticleIndexes &pca,
      const ParticleIndexes &pcb) const override;
  IMP_OBJECT_METHODS(RigidClosePairsFinder);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_RIGID_CLOSE_PAIRS_FINDER_H */
