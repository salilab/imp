/**
 *  \file RigidClosePairsFinder.h
 *  \brief Handle rigid bodies by looking at their members
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_RIGID_CLOSE_PAIRS_FINDER_H
#define IMPCORE_RIGID_CLOSE_PAIRS_FINDER_H

#include "ClosePairsFinder.h"
#include "internal/rigid_pair_score.h"

IMPCORE_BEGIN_NAMESPACE

//! Return close pairs between members of rigid bodies.
/** It needs to be passed another ClosePairsFinder to help
    determine which rigid bodies are close to which others.

    For each pair of RigidBody particles, it returns all pairs
    (p,q), where p and q are taken from different rigid bodies and
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

    \see ClosePairsScoreState
    \see RigidBodyDecorator
    \see cover_members()
 */
class IMPCOREEXPORT RigidClosePairsFinder : public ClosePairsFinder
{
  mutable internal::RigidBodyCollisionData data_;
  Pointer<ClosePairsFinder> cpf_;
  Pointer<FilteredListPairContainer> cpfout_;

  void setup(Particle *) const;
  void setup(const algebra::Sphere3Ds &spheres,
             unsigned int node_index,
             const internal::SphereIndexes &leaves,
             internal::RigidBodyParticleData &data) const;
  Particle *get_member(Particle *a, unsigned int i) const;
  const algebra::Sphere3D get_transformed(Particle *a,
                                          const algebra::Sphere3D &s) const;
  void process_one(Particle *a, Particle *b,
                   FilteredListPairContainer *out,
                   const internal::RigidBodyParticleData &da,
                   const internal::RigidBodyParticleData &db,
                   unsigned int ci,
                   unsigned int cj,
                   std::vector<std::pair<int, int> > &stack) const;
 public:
  //! Use the default choice for the ClosePairsFinder
  RigidClosePairsFinder();
  RigidClosePairsFinder(ClosePairsFinder *cpf);
  ~RigidClosePairsFinder();

  void add_close_pairs(SingletonContainer *pc,
                       FilteredListPairContainer *out) const;

  void add_close_pairs(SingletonContainer *pca,
                       SingletonContainer *pcb,
                       FilteredListPairContainer *out) const;
  void add_close_pairs(Particle *a, Particle *b,
                       FilteredListPairContainer *out) const;

  void show(std::ostream &out= std::cout) const {
    out << "RigidClosePairsFinder" << std::endl;
  }
  VersionInfo get_version_info() const {
    return internal::version_info;
  }

  void set_distance(double d) {
    data_.clear();
    cpf_->set_distance(d);
    ClosePairsFinder::set_distance(d);
  }
  void set_radius_key(FloatKey d) {
    data_.clear();
    cpf_->set_radius_key(d);
    ClosePairsFinder::set_radius_key(d);
  }
#ifndef IMP_DOXYGEN
  std::vector<algebra::Sphere3D> get_tree(Particle *p) const;
  void show_tree(Particle *p, std::ostream &out=std::cout) const;
#endif
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_RIGID_CLOSE_PAIRS_FINDER_H */
