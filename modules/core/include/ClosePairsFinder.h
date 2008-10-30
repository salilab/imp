/**
 *  \file ClosePairsFinder.h
 *  \brief A base class for algorithms to detect proximities.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_CLOSE_PAIRS_FINDER_H
#define IMPCORE_CLOSE_PAIRS_FINDER_H

#include "core_exports.h"
#include "XYZRDecorator.h"
#include "ParticleContainer.h"
#include "FilteredListParticlePairContainer.h"

#include <IMP/RefCountedObject.h>

IMPCORE_BEGIN_NAMESPACE

// to make SWIG happy
class FilteredListParticlePairContainer;
class ParticleContainer;

//! A base class for algorithms to find spatial proximities
/** In general, the algorithm should make sure it returns all
    pairs of appropriate objects which are within get_distance()
    of one another (including the radius).
 */
class IMPCOREEXPORT ClosePairsFinder : public RefCountedObject
{
  FloatKey rk_;
  Float distance_;
 protected:
  Float get_radius(Particle *p) const {
    if (rk_ != FloatKey() && p->has_attribute(rk_)) {
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
      distance(XYZRDecorator(p0, get_radius()), XYZRDecorator(p1, get_radius()))
      is less than get_distance(). Any particle without radius is assumed
      to have a radius of 0. Other pairs can be added too. */
  void add_close_pairs(ParticleContainer *pc,
                          FilteredListParticlePairContainer *out);

  //!
  /** \brief Compute all nearby pairs of particles with the first taken from
      pca and the second from pcb.

      See evaluate(ParticleContainer* for more
      details.
  */
  void add_close_pairs(ParticleContainer *pca,
                          ParticleContainer *pcb,
                          FilteredListParticlePairContainer *out);

  FloatKey get_radius_key() const {
    return rk_;
  }
  void set_radius_key(FloatKey rk) {
    rk_=rk;
  }

  Float get_distance() const {
    return distance_;
  }
  void set_distance(Float distance) {
    distance_=distance;
  }
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CLOSE_PAIRS_FINDER_H */
