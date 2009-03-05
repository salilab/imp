/**
 *  \file atom/BondSingletonScore.h
 *  \brief A Score on the distance between a the two particles in a bond.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPATOM_BOND_SINGLETON_SCORE_H
#define IMPATOM_BOND_SINGLETON_SCORE_H

#include "config.h"

#include <IMP/SingletonScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>

IMPATOM_BEGIN_NAMESPACE

//! Score the bond based on a UnaryFunction,
/** The score is based on the difference between the stored length
    and the actual length and scaled by the stiffness. That is
    stiffness * (distance-bond_length)
    \ingroup bond
    \see BondDecorator
    \see BondedDecorator
 */
class IMPATOMEXPORT BondSingletonScore : public SingletonScore
{
  Pointer<UnaryFunction> f_;
public:
  //! Use f to penalize deviations in length
  BondSingletonScore(UnaryFunction *f);
  virtual ~BondSingletonScore(){}
  virtual Float evaluate(Particle *a,
                         DerivativeAccumulator *da) const;
  virtual void show(std::ostream &out=std::cout) const;
};

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_BOND_SINGLETON_SCORE_H */
