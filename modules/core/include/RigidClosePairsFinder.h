/**
 *  \file RigidClosePairsFinder.h
 *  \brief Handle rigid bodies by looking at their members
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_RIGID_CLOSE_PAIRS_FINDER_H
#define IMPCORE_RIGID_CLOSE_PAIRS_FINDER_H

#include "ClosePairsFinder.h"
#include "rigid_bodies.h"
#include "internal/CoreListSingletonContainer.h"
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

    \note The bounding spheres are kept in internal coordinates for
    the rigid body and transformed on the fly. It would probably be
    faster to cache the tranformed results.

    \note The particles are divided up using a grid. The number of
    grid cells to use should be explored. In addition, with highly
    excentric sets of points, there will be too many cells.

    \pythonexample{rigid_collisions}

    \uses{class RigidClosePairsFinder, CGAL}
    \see ClosePairsScoreState
    \see RigidBody
    \see cover_members()
 */
class IMPCOREEXPORT RigidClosePairsFinder : public ClosePairsFinder
{
  mutable IMP::internal::OwnerPointer<ClosePairsFinder> cpf_;
  IMP::internal::OwnerPointer<Refiner> r_;
  ObjectKey k_;
 public:
#ifndef IMP_DOXYGEN
  //! Use the default choice for the ClosePairsFinder
  /** Use rep to generate the list of representation particles. */
  RigidClosePairsFinder(Refiner *r);
  RigidClosePairsFinder(ClosePairsFinder *cpf,
                        Refiner *r);
#endif
  RigidClosePairsFinder(ClosePairsFinder *cpf=NULL);

  ParticlePairsTemp get_close_pairs(Particle *a, Particle *b,
                                    const ParticlesTemp &pa,
                                    const ParticlesTemp &pb) const;

  void set_distance(double d) {
    cpf_->set_distance(d);
    ClosePairsFinder::set_distance(d);
  }
#if 0
  /** Return a pair of spheres which captures the interaction between the two,
      typically rigid (but not necessarily) particles. The spheres are the
      lowest point down the trees which cover all interactions between the
      two.
   */
  std::pair<algebra::Sphere3D, algebra::Sphere3D>
    get_close_sphere_pair(Particle *a, Particle *b) const;
#endif

#ifndef IMP_SWIG
  internal::MovedSingletonContainer *
    get_moved_singleton_container(SingletonContainer *c,
                                  Model *m, double thresold) const;
#endif
  IMP_CLOSE_PAIRS_FINDER(RigidClosePairsFinder);

};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_RIGID_CLOSE_PAIRS_FINDER_H */
