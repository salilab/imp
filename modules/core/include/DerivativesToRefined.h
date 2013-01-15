/**
 *  \file IMP/core/DerivativesToRefined.h
 *  \brief Accumulate the derivatives of the refined particles
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_DERIVATIVES_TO_REFINED_H
#define IMPCORE_DERIVATIVES_TO_REFINED_H

#include <IMP/core/core_config.h>

#include "XYZ.h"
#include <IMP/Refiner.h>
#include <IMP/singleton_macros.h>
#include <IMP/Pointer.h>
#include <IMP/SingletonDerivativeModifier.h>

IMPCORE_BEGIN_NAMESPACE

//! Copy the derivatives from the particle to its refined particles.
/** \see DerivatvesFromRefinedSingletonModifier
    \see CoverRefined
    \see CentroidOfRefinedSingletonModifer
 An example showing a how to use such a score state to maintain a cover
 of the atoms of a protein by a sphere per residue.
 \pythonexample{cover_particles}
 */
class IMPCOREEXPORT DerivativesToRefined:
public SingletonDerivativeModifier
{
  IMP::OwnerPointer<Refiner> refiner_;
  FloatKeys ks_;
public:
  //! Copy ks to the particles returned by r.
  DerivativesToRefined(Refiner *r,
                       FloatKeys ks
                       = XYZ::get_xyz_keys());

  IMP_INDEX_SINGLETON_DERIVATIVE_MODIFIER(DerivativesToRefined);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_DERIVATIVES_TO_REFINED_H */
