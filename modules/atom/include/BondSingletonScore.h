/**
 *  \file IMP/atom/BondSingletonScore.h
 *  \brief A Score on the distance between a the two particles in a bond.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_BOND_SINGLETON_SCORE_H
#define IMPATOM_BOND_SINGLETON_SCORE_H

#include <IMP/atom/atom_config.h>
#include "bond_decorators.h"
#include <IMP/SingletonScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/base/Pointer.h>

IMPATOM_BEGIN_NAMESPACE

//! Score the bond based on a UnaryFunction,
/** The score is based on the difference between the stored length
    and the actual length and scaled by the stiffness. That is
    stiffness * (distance-bond_length)
    \ingroup bond
    \see Bond
    \see Bonded
 */
class IMPATOMEXPORT BondSingletonScore : public SingletonScore {
  IMP::base::PointerMember<UnaryFunction> f_;

 public:
  //! Use f to penalize deviations in length
  BondSingletonScore(UnaryFunction *f);
  UnaryFunction *get_unary_function() const { return f_; }
  virtual double evaluate_index(Model *m, kernel::ParticleIndex p,
                                DerivativeAccumulator *da) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(Model *m,
                                         const kernel::ParticleIndexes &pis) const
      IMP_OVERRIDE;
  IMP_SINGLETON_SCORE_METHODS(BondSingletonScore);
  IMP_OBJECT_METHODS(BondSingletonScore);
};

IMP_OBJECTS(BondSingletonScore, BondSingletonScores);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_BOND_SINGLETON_SCORE_H */
