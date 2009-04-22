/**
 *  \file ClosePairsFinder.h
 *  \brief A base class for algorithms to detect proximities.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_CLOSE_PAIRS_FINDER_H
#define IMPCORE_CLOSE_PAIRS_FINDER_H

#include "config.h"
#include "XYZRDecorator.h"
#include "FilteredListPairContainer.h"

#include <IMP/RefCounted.h>
#include <IMP/SingletonContainer.h>

IMPCORE_BEGIN_NAMESPACE

// to make SWIG happy
class FilteredListPairContainer;

//! A base class for algorithms to find spatial proximities
/** In general, the algorithm should make sure it returns all
    pairs of appropriate objects which are within distance
    of one another (including the radius).
    \see ClosePairsScoreState
    \see CloseBipartitlePairsScoreState
 */
class IMPCOREEXPORT ClosePairsFinder : public Object
{
  FloatKey rk_;
  double distance_;
 protected:
  Float get_radius(Particle *p) const {
    if (rk_ != FloatKey()) {
      return p->get_value(rk_);
    } else {
      return 0;
    }
  }

 public:
  ClosePairsFinder();
  ~ClosePairsFinder();

  //! Compute all nearby pairs of particles in pc
  /** All pairs of distinct particles, p0, p1, taken from pc such that
      distance(XYZRDecorator(p0, radius_key), XYZRDecorator(p1, radius_key))
      is less than distance. If radius_key is FloatKey() all radii
      are assumed to be 0. Other pairs can be added too. */
  virtual void add_close_pairs(SingletonContainer *pc,
                               FilteredListPairContainer *out) const =0;

  /** \brief Compute all nearby pairs of particles with the first taken from
      pca and the second from pcb.

      See evaluate(SingletonContainer* for more
      details.
  */
  virtual void add_close_pairs(SingletonContainer *pca,
                       SingletonContainer *pcb,
                       FilteredListPairContainer *out) const =0;

  virtual void set_radius_key(FloatKey rk) {
    rk_=rk;
  }
  virtual void set_distance(double d) {
    IMP_check(d>=0, "Distance cannot be negative " << d,
              ValueException);
    distance_=d;
  }
  FloatKey get_radius_key() const {
    return rk_;
  }
  double get_distance() const {
    return distance_;
  }
};

IMP_OUTPUT_OPERATOR(ClosePairsFinder);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CLOSE_PAIRS_FINDER_H */
