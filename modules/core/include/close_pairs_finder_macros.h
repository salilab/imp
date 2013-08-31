/**
 *  \file IMP/core/close_pairs_finder_macros.h
 *  \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_CLOSE_PAIRS_FINDER_MACROS_H
#define IMPCORE_CLOSE_PAIRS_FINDER_MACROS_H

#include "ClosePairsFinder.h"
#include <IMP/base/doxygen_macros.h>
#include <IMP/base/object_macros.h>

/** \deprecated_at{2.1} Declare the methods yourself.
 */
#define IMP_CLOSE_PAIRS_FINDER(Name)                                         \
  IMPCORE_DEPRECATED_MACRO(2.1, "Declare the methods directly.");            \
  virtual kernel::ParticlePairsTemp get_close_pairs(const kernel::ParticlesTemp &pa) const   \
      IMP_OVERRIDE;                                                          \
  virtual kernel::ParticlePairsTemp get_close_pairs(                                 \
      const kernel::ParticlesTemp &pa, const kernel::ParticlesTemp &pb) const IMP_OVERRIDE;  \
  virtual IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bbs) const \
      IMP_OVERRIDE;                                                          \
  virtual IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bas,       \
                                   const algebra::BoundingBox3Ds &bbs) const \
      IMP_OVERRIDE;                                                          \
  using ClosePairsFinder::get_close_pairs;                                   \
  virtual ModelObjectsTemp do_get_inputs(                                    \
      Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;              \
  IMP_OBJECT_METHODS(Name)

#endif /* IMPCORE_CLOSE_PAIRS_FINDER_MACROS_H */
