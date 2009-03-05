/**
 *  \file BondSingletonScore.h
 *  \brief A Score on the distance between a the two particles in a bond.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_BOND_SINGLETON_SCORE_H
#define IMPCORE_BOND_SINGLETON_SCORE_H

#include "config.h"

#include <IMP/SingletonScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>

IMPCORE_BEGIN_NAMESPACE

#ifndef IMP_NO_DEPRECATED

//! Score the bond based on a UnaryFunction,
/** The score is based on the difference between the stored length
    and the actual length and scaled by the stiffness. That is
    stiffness * (distance-bond_length)
    \ingroup bond
    \deprecated use atom::BondSingletonScore instead
    \see BondDecorator
    \see BondedDecorator
 */
class IMPCOREEXPORT BondSingletonScore : public SingletonScore
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

#endif // IMP_NO_DEPRECATED

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BOND_SINGLETON_SCORE_H */
