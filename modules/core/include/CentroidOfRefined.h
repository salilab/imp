/**
 *  \file IMP/core/CentroidOfRefined.h
 *  \brief Set the coordinates of the particle to be the centoid of the
 *  refined particles.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_CENTROID_OF_REFINED_H
#define IMPCORE_CENTROID_OF_REFINED_H

#include <IMP/core/core_config.h>
#include "XYZ.h"
#include <IMP/Refiner.h>
#include <IMP/macros.h>
#include <IMP/base/Pointer.h>
#include <IMP/Decorator.h>
#include <IMP/SingletonModifier.h>
#include "DerivativesToRefined.h"

IMPCORE_BEGIN_NAMESPACE

//! Set the coordinates (or any float keys) of the particle to be the centoid of
//! the corresponding keys in a set of fine particles.
/** An exception is thrown if there are no refined particles. The weight
    FloatKey can be used to reweight the centroid computation, or it can be set
    to FloatKey() and then all the weights will be equal to 1.0.

    \see CoverRefined
    \see DerivativesToRefined
    \see DerivativesFromRefined
 */
class IMPCOREEXPORT CentroidOfRefined : public SingletonModifier {
  base::PointerMember<Refiner> refiner_;
  FloatKeys ks_;
  FloatKey w_;

 public:
  //! Set the keys ks to be the average of the refined particles.
  /**
     @param r refiner used to retrieve the set of fine particles
     @param weight a key used to reweight the contribution of each
                    fine particle to the centroid (eg, mass). In the
                    centroid, this key will be set by apply_index() to
                    the sum of all weights. A default weight of 1.0 is
                    used if weight equals FloatKey().
     @param ks the keys over which the centroid is calculated
   */
  CentroidOfRefined(Refiner *r, FloatKey weight = FloatKey(),
                    FloatKeys ks = XYZ::get_xyz_keys());

  /**
     Compute the weighted centroid of the keys of refine particles (as passed in
     the constructor), and store the centroid values in the corresponding keys of
     particle pi. The sum of all weight of the refined particles are stored
     in the corresponding weight key of pi (it is assumed that pi has this key).

     @param m the model of pi
     @param pi the centroid particle
   */
  virtual void apply_index(kernel::Model *m, kernel::ParticleIndex pi) const
      IMP_OVERRIDE;

  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;

  virtual kernel::ModelObjectsTemp do_get_outputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;

  IMP_SINGLETON_MODIFIER_METHODS(CentroidOfRefined);

  IMP_OBJECT_METHODS(CentroidOfRefined);
};

//! A particle that is the geometric centroid of other particles.
/** A decorator which constrains a particle to be the centroid of a
    set of other particles. The centroid is updated before model
    evaluation and its derivatives are copied to its children,
    using a constraint that is created at setup time.

    \usesconstraint
*/
IMP_SUMMARIZE_DECORATOR_DECL(Centroid, XYZ, XYZs,
                             Centroid is computed before each evaluation);


IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CENTROID_OF_REFINED_H */
