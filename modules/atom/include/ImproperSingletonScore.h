/**
 *  \file IMP/atom/ImproperSingletonScore.h
 *  \brief A score on the deviation of an improper angle from ideality.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_IMPROPER_SINGLETON_SCORE_H
#define IMPATOM_IMPROPER_SINGLETON_SCORE_H

#include <IMP/atom/atom_config.h>
#include "bond_decorators.h"
#include <IMP/SingletonScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/base/Pointer.h>

IMPATOM_BEGIN_NAMESPACE

//! Score the improper dihedral based on a UnaryFunction,
/** This scores the improper dihedral using information stored in its
    Dihedral decorator. The score is based on the difference between the
    stored ideal improper angle and the actual angle and scaled by the
    stiffness. That is stiffness * (improper_angle-ideal_value). The
    difference is in radians between -pi and +pi; it is the shortest
    distance from one angle to the other.

    \note The multiplicity of the the Dihedral is not used.

    \see CHARMMTopology::add_impropers(), Dihedral.
 */
class IMPATOMEXPORT ImproperSingletonScore : public SingletonScore {
  IMP::base::PointerMember<UnaryFunction> f_;

 public:
  //! Use f to penalize deviations in angle
  ImproperSingletonScore(UnaryFunction *f);
  virtual double evaluate_index(kernel::Model *m, kernel::ParticleIndex p,
                                DerivativeAccumulator *da) const IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_SINGLETON_SCORE_METHODS(ImproperSingletonScore);
  IMP_OBJECT_METHODS(ImproperSingletonScore);
  ;
};

IMP_OBJECTS(ImproperSingletonScore, ImproperSingletonScores);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_IMPROPER_SINGLETON_SCORE_H */
