/**
 *  \file ClosePairsFinder.h
 *  \brief A base class for algorithms to detect proximities.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_CLOSE_PAIRS_FINDER_H
#define IMPCORE_CLOSE_PAIRS_FINDER_H

#include "config.h"
#include "XYZR.h"
#include "ListPairContainer.h"
#include "internal/MovedSingletonContainer.h"

#include <IMP/RefCounted.h>
#include <IMP/SingletonContainer.h>

IMPCORE_BEGIN_NAMESPACE

// to make SWIG happy
class ListPairContainer;

//! A base class for algorithms to find spatial proximities
/** In general, the algorithm should make sure it returns all
    pairs of appropriate objects which are within the distance
    of one another (including their radius). They are free to
    return more if it is conventient, but this is not
    recommended as the list can easily become very large.
    \see ClosePairsScoreState
    \see CloseBipartitlePairsScoreState
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
  ClosePairsFinder();
  ~ClosePairsFinder();

  /** \name Methods to find close pairs
      The methods add apprioriately close pairs of particles from the
      input list (or lists, for the bipartite version) to the out
      list.
      @{
   */
  virtual ParticlePairsTemp get_close_pairs(SingletonContainer *pc) const =0;

  virtual ParticlePairsTemp get_close_pairs(SingletonContainer *pca,
                                            SingletonContainer *pcb) const =0;
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
  virtual ParticlesTemp get_input_particles(SingletonContainer *pc) const=0;
  virtual ParticlesTemp get_input_particles(SingletonContainer *a,
                                            SingletonContainer *b) const=0;
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
