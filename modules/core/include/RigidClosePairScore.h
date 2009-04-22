/**
 *  \file RigidClosePairScore.h
 *  \brief A pair score to efficient find close pairs between two
 *  rigid bodies.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_RIGID_CLOSE_PAIR_SCORE_H
#define IMPCORE_RIGID_CLOSE_PAIR_SCORE_H

#include "config.h"

#include "internal/version_info.h"
#include "rigid_bodies.h"
#include "internal/rigid_pair_score.h"

IMPCORE_BEGIN_NAMESPACE

//! Compute collisions between the members of two rigid bodies
/** This pair score calls a passed PairScore on each RigidMember particle
    pair (p,q), one taken from the first pass RigidBody particle and the
    second from the second RigidBody particle such that
    \code
    distance(XYZRDecorator(p, radius_key), XYZRDecorator(p, radius_key)) < 0
    \endcode
    A called particle that is not an RigidBody particle is assumed to
    be a RigidBody consisting of a single sphere.

    The user must ensure that the RigidBody particle is assigned a radius
    that encloses all of its RigidMember particles.

    \par Algorithmic details:
    For each rigid body seen, a sphere hierarchy is built enclosing the
    spheres for each RigidMember particle. When it is called with two
    particles it walks down the hierarchy and applies the passed
    PairScore to each intersecting pair of RigidMember particles. Having
    CGAL makes the computations more efficient.

    \note The bounding spheres are kept in internal coordinates for
    the rigid body and transformed on the fly. It would probably be
    faster to cache the tranformed results. Unfortunately, this is
    hard to do as the PairScore can't tall when you move on to a new
    round. One solution would be to make the iteration count in model
    public. Then one could store the iteration cound with the cached
    results and invalidate the cache if needed.

    \note The particles are divided up using a grid. The number of
    grid cells to use should be explored. In addition, with highly
    excentric sets of points, there will be too many cells.
 */
class IMPCOREEXPORT RigidClosePairScore: public PairScore {
  mutable internal::RigidBodyCollisionData data_;
  Pointer<PairScore> ps_;
  FloatKey rk_;
  double threshold_;
  mutable std::vector<std::pair<algebra::Sphere3D, algebra::Sphere3D> >
    last_pairs_;
  mutable Pointer<Particle> lp0_, lp1_;

  void setup(Particle *) const;
  void setup(const algebra::Sphere3Ds &spheres,
             unsigned int node_index,
             const internal::SphereIndexes &leaves,
             internal::RigidBodyParticleData &data) const;
  double process(Particle *a, Particle *b,
                 DerivativeAccumulator *da) const;
  Particle *get_member(Particle *a, unsigned int i) const;
  const algebra::Sphere3D get_transformed(Particle *a,
                                          const algebra::Sphere3D &s) const;
public:
  RigidClosePairScore(PairScore *applied,
                      double threshold,
                      FloatKey radius_key
                      = XYZRDecorator::get_default_radius_key());
#ifndef IMP_DOXYGEN
  std::vector<std::pair<algebra::Sphere3D, algebra::Sphere3D> >
                 get_last_sphere_pairs() const;
  std::vector<algebra::Sphere3D> get_tree(Particle *p) const;
  void show_tree(Particle *p, std::ostream &out=std::cout) const;
#endif
  IMP_PAIR_SCORE(RigidClosePairScore, internal::version_info)
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_RIGID_CLOSE_PAIR_SCORE_H */
