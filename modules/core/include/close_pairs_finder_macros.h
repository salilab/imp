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

// various installs of doxygen get messed up with this macro, so hide it
#ifdef IMP_DOXYGEN
/** Helper macro for implementing
    IMP::core::ClosePairsFinder objects. In addition to the
    IMP_OBJECT methods it declares:
    - IMP::core::get_close_pairs()
*/
#define IMP_CLOSE_PAIRS_FINDER(Name)
#else
#define IMP_CLOSE_PAIRS_FINDER(Name)                                       \
  IMP_IMPLEMENT(                                                           \
      ParticlePairsTemp get_close_pairs(const ParticlesTemp &pa) const);   \
  IMP_IMPLEMENT(ParticlePairsTemp get_close_pairs(                         \
      const ParticlesTemp &pa, const ParticlesTemp &pb) const);            \
  IMP_IMPLEMENT(                                                           \
      IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bbs) const); \
  IMP_IMPLEMENT(                                                           \
      IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bas,         \
                               const algebra::BoundingBox3Ds &bbs) const); \
  using ClosePairsFinder::get_close_pairs;                                 \
  IMP_IMPLEMENT(ModelObjectsTemp do_get_inputs(                            \
      Model *m, const ParticleIndexes &pis) const);                        \
  IMP_OBJECT(Name)
#endif

#endif /* IMPCORE_CLOSE_PAIRS_FINDER_MACROS_H */
