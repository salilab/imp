/**
 *  \file RigidClosePairsFinder.h
 *  \brief Handle rigid bodies by looking at their members
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_RIGID_CLOSE_PAIRS_FINDER_H
#define IMPCORE_RIGID_CLOSE_PAIRS_FINDER_H

#include "ClosePairsFinder.h"
#include "rigid_bodies.h"
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

    \note Having CGAL makes the computations more efficient.

    \note The bounding spheres are kept in internal coordinates for
    the rigid body and transformed on the fly. It would probably be
    faster to cache the tranformed results.

    \note The particles are divided up using a grid. The number of
    grid cells to use should be explored. In addition, with highly
    excentric sets of points, there will be too many cells.

    \htmlinclude rigid_collisions.py.html

    \note This class uses the IMP::core::BoxSweepClosePairsFinder by
    default if \ref CGAL "CGAL" is available.

    \ingroup CGAL
    \see ClosePairsScoreState
    \see RigidBody
    \see cover_members()
 */
class IMPCOREEXPORT RigidClosePairsFinder : public ClosePairsFinder
{
  IMP::internal::OwnerPointer<ClosePairsFinder> cpf_;
  IMP::internal::OwnerPointer<Refiner> r_;
  ObjectKey k_;
 public:
  //! Use the default choice for the ClosePairsFinder
  /** Use rep to generate the list of representation particles. */
  RigidClosePairsFinder(Refiner *r);
  RigidClosePairsFinder(ClosePairsFinder *cpf,
                        Refiner *r);
  RigidClosePairsFinder();
  RigidClosePairsFinder(ClosePairsFinder *cpf);
  IMP_CLOSE_PAIRS_FINDER(RigidClosePairsFinder, get_module_version_info());

  ParticlePairsTemp get_close_pairs(Particle *a, Particle *b) const;

  void set_distance(double d) {
    cpf_->set_distance(d);
    ClosePairsFinder::set_distance(d);
  }
#ifndef IMP_SWIG
  internal::MovedSingletonContainer *
    get_moved_singleton_container(SingletonContainer *c,
                                  Model *m, double thresold) const;
#endif
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_RIGID_CLOSE_PAIRS_FINDER_H */
