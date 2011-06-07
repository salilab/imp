/**
 *  \file ClosePairsFinder.h
 *  \brief A base class for algorithms to detect proximities.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_CLOSE_PAIRS_FINDER_H
#define IMPCORE_CLOSE_PAIRS_FINDER_H

#include "core_config.h"
#include "XYZR.h"
#include "internal/MovedSingletonContainer.h"

#include <IMP/RefCounted.h>
#include <IMP/SingletonContainer.h>

IMPCORE_BEGIN_NAMESPACE


//! A base class for algorithms to find spatial proximities
/** In general, the algorithm should make sure it returns all
    pairs of appropriate objects which are within the distance
    of one another (including their radius). They are free to
    return more if it is convenient, but this is not
    recommended as the list can easily become very large.
    \see ClosePairContainer
    \see CloseBipartitePairContainer
 */
class IMPCOREEXPORT ClosePairsFinder : public Object
{
  double distance_;
 protected:
  //! Get the radius if get_radius_key() is non-default, otherwise 0.
  Float get_radius(Particle *p) const {
    return XYZR(p).get_radius();
  }

 public:
  ClosePairsFinder(std::string name);
  ~ClosePairsFinder();

  //! Return whether the two particles are close
  bool get_are_close(Particle *a, Particle *b) const {
      XYZ da(a);
      XYZ db(b);
      Float ra= XYZR(a).get_radius();
      Float rb= XYZR(b).get_radius();
      Float sr= ra+rb+get_distance();
      for (unsigned int i=0; i< 3; ++i) {
        double delta=std::abs(da.get_coordinate(i) - db.get_coordinate(i));
        if (delta >= sr) {
          return false;
        }
      }
      return get_interiors_intersect(algebra::SphereD<3>(da.get_coordinates(),
                                                         ra+get_distance()),
                                     algebra::SphereD<3>(db.get_coordinates(),
                                                         rb));
  }

  /** \name Methods to find close pairs
      The methods add appropriately close pairs of particles from the
      input list (or lists, for the bipartite version) to the out
      list. The ones that take bounding boxes return pairs of ints
      indexing the input list(s).
      @{
   */
  virtual ParticlePairsTemp get_close_pairs(SingletonContainer *pc) const {
    return get_close_pairs(pc->get_particles());
  }

  virtual ParticlePairsTemp get_close_pairs(SingletonContainer *pca,
                                    SingletonContainer *pcb) {
    return get_close_pairs(pca->get_particles(),
                           pcb->get_particles());
  }
  virtual ParticlePairsTemp get_close_pairs(const ParticlesTemp &pc) const;
  virtual ParticlePairsTemp get_close_pairs(const ParticlesTemp &pca,
                                            const ParticlesTemp &pcb) const;

  virtual IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bbs) const;
  virtual IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bas,
                                   const algebra::BoundingBox3Ds &bbs) const;
  /** @} */

  /** \name The distance threshold
      All pairs within this distance threshold are added to the output
      list.
      @{
  */
  virtual void set_distance(double d) {
    distance_=d;
  }
  double get_distance() const {
    return distance_;
  }
  /** @} */
  /** \name Used particles

      Return all the particles touched in processing the passed ones.
      @{
  */
  virtual ParticlesTemp get_input_particles(const ParticlesTemp &ps) const;
  virtual ContainersTemp get_input_containers(const ParticlesTemp &ps) const;
  /** @} */
#ifndef SWIG
  /** \brief Return a container which lists all particles which moved more
      than threshold
  */
  virtual internal::MovedSingletonContainer*
    get_moved_singleton_container(SingletonContainer *in,
                                  Model *m,
                                  double threshold)const;
#endif
};

IMP_OUTPUT_OPERATOR(ClosePairsFinder);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CLOSE_PAIRS_FINDER_H */
