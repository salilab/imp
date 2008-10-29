/**
 *  \file BondDecoratorSingletonScore.h
 *  \brief A Score on the distance between a the two particles in a bond.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_BOND_DECORATOR_SINGLETON_SCORE_H
#define IMPCORE_BOND_DECORATOR_SINGLETON_SCORE_H

#include "core_exports.h"

#include <IMP/SingletonScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>

IMPCORE_BEGIN_NAMESPACE

//! Score the bond based on a UnaryFunction,
/** The score is based on the difference between the stored length
    and the actual length and scaled by the stiffness. That is
    stiffness * (distance-bond_length)

    \ingroup singletonscore
 */
class IMPCOREEXPORT BondDecoratorSingletonScore : public SingletonScore
{
  Pointer<UnaryFunction> f_;
public:
  BondDecoratorSingletonScore(UnaryFunction *f);
  virtual ~BondDecoratorSingletonScore(){}
  virtual Float evaluate(Particle *a,
                         DerivativeAccumulator *da) const;
  virtual void show(std::ostream &out=std::cout) const;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BOND_DECORATOR_SINGLETON_SCORE_H */
