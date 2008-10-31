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
    pairs of appropriate objects which are within distance
    of one another (including the radius).
 */
class IMPCOREEXPORT ClosePairsFinder : public RefCountedObject
{
 protected:
  Float get_radius(Particle *p, FloatKey rk) const {
    if (rk != FloatKey()) {
      return p->get_value(rk);
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
  virtual void add_close_pairs(ParticleContainer *pc,
                       Float distance,
                       FloatKey radius_key,
                       FilteredListParticlePairContainer *out) const =0;

  //!
  /** \brief Compute all nearby pairs of particles with the first taken from
      pca and the second from pcb.

      See evaluate(ParticleContainer* for more
      details.
  */
  virtual void add_close_pairs(ParticleContainer *pca,
                       ParticleContainer *pcb,
                       Float distance,
                       FloatKey radius_key,
                       FilteredListParticlePairContainer *out) const =0;

  void show(std::ostream &out=std::cout) const;
};

IMP_OUTPUT_OPERATOR(ClosePairsFinder);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CLOSE_PAIRS_FINDER_H */
