/**
 *  \file CentroidOfRefinedSingletonModifier.h
 *  \brief Set the coordinates of the particle to be the centoid of the
 *  refined particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_CENTROID_OF_REFINED_SINGLETON_MODIFIER_H
#define IMPCORE_CENTROID_OF_REFINED_SINGLETON_MODIFIER_H

#include "config.h"
#include "internal/core_version_info.h"

#include "XYZDecorator.h"
#include <IMP/ParticleRefiner.h>
#include <IMP/Pointer.h>
#include <IMP/SingletonModifier.h>


IMPCORE_BEGIN_NAMESPACE

//! Set the coordinates of the particle to be the centoid of the particles.
/** An exception is thrown if there are no refined particles. The weight
    FloatKey can be FloatKey and then all the weights will be equal.

    \relates CoverRefinedSingletonModifier
 */
class IMPCOREEXPORT CentroidOfRefinedSingletonModifier:
public SingletonModifier
{
  Pointer<ParticleRefiner> r_;
  FloatKeys ks_;
  FloatKey w_;
public:
  //! Set the keys ks to be the average of the refined particles.
  CentroidOfRefinedSingletonModifier(ParticleRefiner *r,
                                     FloatKey weight=FloatKey(),
                                     FloatKeys ks
                                      = XYZDecorator::get_xyz_keys());
  ~CentroidOfRefinedSingletonModifier();

  IMP_SINGLETON_MODIFIER(internal::core_version_info);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CENTROID_OF_REFINED_SINGLETON_MODIFIER_H */
