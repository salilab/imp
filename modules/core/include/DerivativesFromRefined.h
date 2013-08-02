/**
 *  \file IMP/core/DerivativesFromRefined.h
 *  \brief Accumulate the derivatives of the refined particles
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_DERIVATIVES_FROM_REFINED_H
#define IMPCORE_DERIVATIVES_FROM_REFINED_H

#include <IMP/core/core_config.h>

#include "XYZ.h"
#include <IMP/Refiner.h>
#include <IMP/singleton_macros.h>
#include <IMP/base/Pointer.h>
#include <IMP/SingletonDerivativeModifier.h>

IMPCORE_BEGIN_NAMESPACE

//! Accumulate the derivatives of the refined particles.
/** \see DerivativesToRefined
    \see CoverRefined
    \see CentroidOfRefinedSingletonModifer
 An example showing a how to use such a score state to maintain a cover
 of the atoms of a protein by a sphere per residue.
 \include cover_particles.py
 */
class IMPCOREEXPORT DerivativesFromRefined
    : public SingletonDerivativeModifier {
  IMP::base::PointerMember<Refiner> refiner_;
  FloatKeys ks_;

 public:
  //! Copy ks from the particles returned by r.
  DerivativesFromRefined(Refiner *r, FloatKeys ks = XYZ::get_xyz_keys());

  virtual void apply_index(Model *m, ParticleIndex a) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(Model *m,
                                         const ParticleIndexes &pis) const
      IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_outputs(Model *m,
                                          const ParticleIndexes &pis) const
      IMP_OVERRIDE;
  IMP_SINGLETON_MODIFIER_METHODS(DerivativesFromRefined);
  IMP_OBJECT_METHODS(DerivativesFromRefined);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_DERIVATIVES_FROM_REFINED_H */
