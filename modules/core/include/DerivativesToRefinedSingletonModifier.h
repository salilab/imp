/**
 *  \file DerivativesToRefinedSingletonModifier.h
 *  \brief Accumulate the derivatives of the refined particles
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_DERIVATIVES_TO_REFINED_SINGLETON_MODIFIER_H
#define IMPCORE_DERIVATIVES_TO_REFINED_SINGLETON_MODIFIER_H

#include "config.h"
#include "internal/version_info.h"

#include "XYZDecorator.h"
#include <IMP/ParticleRefiner.h>
#include <IMP/Pointer.h>
#include <IMP/SingletonModifier.h>

IMPCORE_BEGIN_NAMESPACE

//! Copy the derivatives from the particle to its refined particles.
/** \see DerivatvesFromRefinedSingletonModifier
    \see CoverRefinedSingletonModifier
    \see CentroidOfRefinedSingletonModifer
 An example showing a how to use such a score state to maintain a cover
 of the atoms of a protein by a sphere per residue.
 \verbinclude simple_examples/cover_particles.py
 */
class IMPCOREEXPORT DerivativesToRefinedSingletonModifier:
public SingletonModifier
{
  Pointer<ParticleRefiner> r_;
  FloatKeys ks_;
public:
  //! Copy ks to the particles returned by r.
  DerivativesToRefinedSingletonModifier(ParticleRefiner *r,
                                        FloatKeys ks
                                        = XYZDecorator::get_xyz_keys());
  ~DerivativesToRefinedSingletonModifier();

  IMP_SINGLETON_MODIFIER_DA(internal::version_info);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_DERIVATIVES_TO_REFINED_SINGLETON_MODIFIER_H */
