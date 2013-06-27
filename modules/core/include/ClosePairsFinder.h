/**
 *  \file IMP/core/ClosePairsFinder.h
 *  \brief A base class for algorithms to detect proximities.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_CLOSE_PAIRS_FINDER_H
#define IMPCORE_CLOSE_PAIRS_FINDER_H

#include <IMP/core/core_config.h>
#include <IMP/PairPredicate.h>

#include <IMP/base/Object.h>
#include <IMP/SingletonContainer.h>
#include <IMP/internal/container_helpers.h>
#include <IMP/model_object_helpers.h>

IMPCORE_BEGIN_NAMESPACE
#ifndef IMP_DOXYGEN
namespace internal {
class MovedSingletonContainer;
}
#endif

//! A base class for algorithms to find spatial proximities
/** In general, the algorithm should make sure it returns all
    pairs of appropriate objects which are within the distance
    of one another (including their radius). They are free to
    return more if it is convenient, but this is not
    recommended as the list can easily become very large.
    \see ClosePairContainer
    \see CloseBipartitePairContainer
 */
class IMPCOREEXPORT ClosePairsFinder : public ParticleInputs,
                                       public IMP::base::Object {
  double distance_;

 public:
  ClosePairsFinder(std::string name);
  ~ClosePairsFinder();

  /** \deprecated_at{2.1} use the index-based on instead. */
  IMPCORE_DEPRECATED_METHOD_DECL(2.1)
  ParticlePairsTemp get_close_pairs(const ParticlesTemp &pc) const;
  /** \deprecated_at{2.1} use the index-based on instead. */
  IMPCORE_DEPRECATED_METHOD_DECL(2.1)
  ParticlePairsTemp get_close_pairs(const ParticlesTemp &pca,
                                    const ParticlesTemp &pcb) const;
  virtual ParticleIndexPairs get_close_pairs(
      Model *m, const ParticleIndexes &pc) const = 0;
  virtual ParticleIndexPairs get_close_pairs(
      Model *m, const ParticleIndexes &pca,
      const ParticleIndexes &pcb) const = 0;
  virtual IntPairs get_close_pairs(
      const algebra::BoundingBox3Ds &bbs) const = 0;
  virtual IntPairs get_close_pairs(
      const algebra::BoundingBox3Ds &bas,
      const algebra::BoundingBox3Ds &bbs) const = 0;
  /** @} */

  /** \name The distance threshold
      All pairs within this distance threshold are added to the output
      list.
      @{
  */
  virtual void set_distance(double d) { distance_ = d; }
  double get_distance() const { return distance_; }
  /** @} */

 public:
  /** @name Methods to control the set of filters

     PairPredicates objects can be used as filters to prevent
     the addition of pairs to the containeroutput list. Pairs
     for which the predicate evaluates to a non-zero value are
     excluded.
  */
  /**@{*/
  IMP_LIST(public, PairFilter, pair_filter, PairPredicate *, PairPredicates);
/**@}*/
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  /** \brief Return a container which lists all particles which moved more
      than threshold
  */
  virtual internal::MovedSingletonContainer *get_moved_singleton_container(
      SingletonContainer *in, double threshold) const;
#endif
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CLOSE_PAIRS_FINDER_H */
