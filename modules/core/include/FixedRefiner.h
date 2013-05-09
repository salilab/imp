/**
 *  \file IMP/core/FixedRefiner.h
 *  \brief A particle refiner which returns a fixed set of particles
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_FIXED_REFINER_H
#define IMPCORE_FIXED_REFINER_H

#include <IMP/core/core_config.h>

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Refiner.h>
#include <IMP/refiner_macros.h>

IMPCORE_BEGIN_NAMESPACE

//! The refiner can refine any particle by returning a fixed set
/**
 */
class IMPCOREEXPORT FixedRefiner : public Refiner {
  ParticlesTemp ps_;

 public:
  //! Store the set of particles
  FixedRefiner(const ParticlesTemp &ps);

  IMP_REFINER(FixedRefiner);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_FIXED_REFINER_H */
