/**
 *  \file CentroidOfRefined.h
 *  \brief Set the coordinates of the particle to be the centoid of the
 *  refined particles.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_CENTROID_OF_REFINED_H
#define IMPCORE_CENTROID_OF_REFINED_H

#include "config.h"
#include "internal/version_info.h"

#include "XYZ.h"
#include <IMP/Refiner.h>
#include <IMP/Pointer.h>
#include <IMP/SingletonModifier.h>


IMPCORE_BEGIN_NAMESPACE

//! Set the coordinates of the particle to be the centoid of the particles.
/** An exception is thrown if there are no refined particles. The weight
    FloatKey can be FloatKey and then all the weights will be equal.

    \see CoverRefined
    \see DerivativesToRefined
    \see DerivativesFromRefined
 */
class IMPCOREEXPORT CentroidOfRefined:
public SingletonModifier
{
  Pointer<Refiner> r_;
  FloatKeys ks_;
  FloatKey w_;
public:
  //! Set the keys ks to be the average of the refined particles.
  CentroidOfRefined(Refiner *r,
                                     FloatKey weight=FloatKey(),
                                     FloatKeys ks
                                      = XYZ::get_xyz_keys());

  IMP_SINGLETON_MODIFIER(CentroidOfRefined,
                         internal::version_info);
};



IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CENTROID_OF_REFINED_H */
