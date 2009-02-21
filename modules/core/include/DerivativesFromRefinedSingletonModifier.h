/**
 *  \file DerivativesFromRefinedSingletonModifier.h
 *  \brief Accumulate the derivatives of the refined particles
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_DERIVATIVES_FROM_REFINED_SINGLETON_MODIFIER_H
#define IMPCORE_DERIVATIVES_FROM_REFINED_SINGLETON_MODIFIER_H

#include "config.h"
#include "internal/version_info.h"

#include "XYZDecorator.h"
#include <IMP/ParticleRefiner.h>
#include <IMP/Pointer.h>
#include <IMP/SingletonModifier.h>


IMPCORE_BEGIN_NAMESPACE

//! Accumulate the derivatives of the refined particles.
/** \see DerivativesToRefinedSingletonModifier
 An example showing a how to use such a score state to maintain a cover
 of the atoms of a protein by a sphere per residue.
 \verbinclude simple_examples/cover_particles.py
 */
class IMPCOREEXPORT DerivativesFromRefinedSingletonModifier:
public SingletonModifier
{
  Pointer<ParticleRefiner> r_;
  FloatKeys ks_;
public:
  //! Copy ks from the particles returned by r.
  DerivativesFromRefinedSingletonModifier(ParticleRefiner *r,
                                          FloatKeys ks
                                           = XYZDecorator::get_xyz_keys());
  ~DerivativesFromRefinedSingletonModifier();

  IMP_SINGLETON_MODIFIER(internal::version_info);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_DERIVATIVES_FROM_REFINED_SINGLETON_MODIFIER_H */
