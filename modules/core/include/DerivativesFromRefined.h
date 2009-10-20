/**
 *  \file DerivativesFromRefined.h
 *  \brief Accumulate the derivatives of the refined particles
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_DERIVATIVES_FROM_REFINED_H
#define IMPCORE_DERIVATIVES_FROM_REFINED_H

#include "config.h"

#include "XYZ.h"
#include <IMP/Refiner.h>
#include <IMP/Pointer.h>
#include <IMP/SingletonModifier.h>


IMPCORE_BEGIN_NAMESPACE

//! Accumulate the derivatives of the refined particles.
/** \see DerivativesToRefined
    \see CoverRefined
    \see CentroidOfRefinedSingletonModifer
 An example showing a how to use such a score state to maintain a cover
 of the atoms of a protein by a sphere per residue.
 \htmlinclude cover_particles.py.html
 */
class IMPCOREEXPORT DerivativesFromRefined:
public SingletonModifier
{
  IMP::internal::OwnerPointer<Refiner> r_;
  FloatKeys ks_;
public:
  //! Copy ks from the particles returned by r.
  DerivativesFromRefined(Refiner *r,
                                          FloatKeys ks
                                           = XYZ::get_xyz_keys());

  IMP_SINGLETON_MODIFIER_DA(DerivativesFromRefined, get_module_version_info());
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_DERIVATIVES_FROM_REFINED_H */
