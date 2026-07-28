/**
 *  \file IMP/core/DerivativesToRefined.h
 *  \brief Accumulate the derivatives of the refined particles
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_DERIVATIVES_TO_REFINED_H
#define IMPCORE_DERIVATIVES_TO_REFINED_H

#include <IMP/core/core_config.h>

#include "XYZ.h"
#include <IMP/Refiner.h>
#include <IMP/singleton_macros.h>
#include <IMP/Pointer.h>
#include <IMP/SingletonModifier.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPCORE_BEGIN_NAMESPACE

//! Copy the derivatives from a coarse particle to its refined particles
/** \see DerivatvesFromRefinedSingletonModifier
    \see CoverRefined
    \see CentroidOfRefinedSingletonModifier
 An example showing a how to use such a score state to maintain a cover
 of the atoms of a protein by a sphere per residue.
 \include cover_particles.py
 */
class IMPCOREEXPORT DerivativesToRefined : public SingletonModifier {
  IMP::PointerMember<Refiner> refiner_;
  FloatKeys ks_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<SingletonModifier>(this), refiner_, ks_);
  }
  IMP_OBJECT_SERIALIZE_DECL(DerivativesToRefined);

 public:
  //! Copy ks to the particles returned by r.
  DerivativesToRefined(Refiner *r, FloatKeys ks = XYZ::get_xyz_keys());

  DerivativesToRefined() {}

  virtual void apply_index(Model *m, ParticleIndex a) const
      override;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override;
  virtual ModelObjectsTemp do_get_outputs(
      Model *m, const ParticleIndexes &pis) const override;
  IMP_SINGLETON_MODIFIER_METHODS(DerivativesToRefined);
  IMP_OBJECT_METHODS(DerivativesToRefined);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_DERIVATIVES_TO_REFINED_H */
