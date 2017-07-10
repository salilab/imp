/**
 *  \file IMP/spb/TiltSingletonScore.h
 *  \brief A Score on the distance to a fixed point.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 */

#ifndef IMPSPB_TILT_SINGLETON_SCORE_H
#define IMPSPB_TILT_SINGLETON_SCORE_H

#include <IMP/Pointer.h>
#include <IMP/SingletonScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/singleton_macros.h>
#include "spb_config.h"

IMPSPB_BEGIN_NAMESPACE

//! Apply a function to the distance to a fixed point.
/** A particle is scored based on the distance between it and a constant
    point as passed to a UnaryFunction. This is useful for anchoring
    constraining particles within a sphere.

    To restrain a set of particles store in SingletonContainer pc in a sphere
    do the following:
    \htmlinclude restrain_in_sphere.py
 */
class IMPSPBEXPORT TiltSingletonScore : public SingletonScore {
  IMP::PointerMember<UnaryFunction> f_;
  algebra::VectorD<3> local_;
  algebra::VectorD<3> global_;

 public:
  TiltSingletonScore(UnaryFunction *f, const algebra::VectorD<3> &v1,
                     const algebra::VectorD<3> &v2);

  virtual double evaluate_index(IMP::Model *m, const IMP::ParticleIndex p,
                                DerivativeAccumulator *da) const IMP_OVERRIDE;

  virtual IMP::ModelObjectsTemp do_get_inputs(
      IMP::Model *m, const IMP::ParticleIndexes &pis) const IMP_OVERRIDE;

  IMP_SINGLETON_SCORE_METHODS(TiltSingletonScore);
  IMP_OBJECT_METHODS(TiltSingletonScore);
  IMP_SHOWABLE(TiltSingletonScore);
  // IMP_SIMPLE_SINGLETON_SCORE(TiltSingletonScore);
};

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_TILT_SINGLETON_SCORE_H */
