/**
 *  \file IMP/core/WeightedDerivativesToRefined.h
 *  \brief Copies derivatives from coarse grained particle
 *         to its refined set of particles using weights
 *         indicated by a weight key.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_WEIGHTED_DERIVATIVES_TO_REFINED_H
#define IMPCORE_WEIGHTED_DERIVATIVES_TO_REFINED_H

#include <IMP/core/core_config.h>

#include "XYZ.h"
#include <IMP/Refiner.h>
#include <IMP/singleton_macros.h>
#include <IMP/base/Pointer.h>
#include <IMP/SingletonDerivativeModifier.h>

IMPCORE_BEGIN_NAMESPACE

//! Copy the derivatives from a coarse particle to its refined particles
/** \see DerivatvesFromRefined
    \see DerivatvesToRefined
    \see Centroid
    \see atom::CenterOfMass
 */
class IMPCOREEXPORT WeightedDerivativesToRefined :
public SingletonDerivativeModifier {
  IMP::base::PointerMember<Refiner> refiner_;
  FloatKey w_; // weights key
  FloatKeys keys_;

 public:
  //! Constructs a weighted derivatives-to-refined modifier that copies
  //! derivatives from the coarse particle to its refinement
  /**
     Constructs a weighted derivatives-to-refined modifier that copies
     derivatives from the coarse particle to its refinement. The modifier
     copy the derivatives of keys to the particles returned by r, using weights w,
     divided by weight of pi.

     @param r a refiner to get fine particles for particle
     @param w the key for the weight given to derivatives of the fine
              particle (to be divided by the weight of the coarse
              particle). If w equals FloatKey(), then a default weight
              of [1.0 / N] is used, for N equals the number of particles
              returned by r.
     @param keys the keys whose derivatives are to be copied.
   */
  WeightedDerivativesToRefined(Refiner *r,
                               FloatKey w = FloatKey(),
                               FloatKeys keys = XYZ::get_xyz_keys());

  //! Copies weighted derivatives to fine particles
  /**
     Copy ks to the particles returned by the refiner r, using weights w,
     divided by weight of pi, all as indicated in the constructor

     @param m the model
     @param pi the coarse grained particle
  */
  virtual void apply_index(kernel::Model *m, kernel::ParticleIndex pi) const
      IMP_OVERRIDE;

  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;

  virtual kernel::ModelObjectsTemp do_get_outputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;

  IMP_SINGLETON_MODIFIER_METHODS(WeightedDerivativesToRefined);

  IMP_OBJECT_METHODS(WeightedDerivativesToRefined);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_WEIGHTED_DERIVATIVES_TO_REFINED_H */
