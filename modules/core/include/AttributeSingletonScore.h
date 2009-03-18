/**
 *  \file AttributeSingletonScore.h
 *  \brief A score based on the unmodified value of an attribute.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_ATTRIBUTE_SINGLETON_SCORE_H
#define IMPCORE_ATTRIBUTE_SINGLETON_SCORE_H

#include "config.h"

#include <IMP/SingletonScore.h>
#include <IMP/Pointer.h>
#include <IMP/UnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE

//! Apply a function to an attribute.
/** This Score scores a particle by passing an attribute value directly
    to a UnaryFunction.
 */
class IMPCOREEXPORT AttributeSingletonScore : public SingletonScore
{
  Pointer<UnaryFunction> f_;
  FloatKey k_;
public:
  //! Apply function f to attribete k
  AttributeSingletonScore(UnaryFunction *f, FloatKey k);
  virtual ~AttributeSingletonScore(){}
  virtual Float evaluate(Particle *a,
                         DerivativeAccumulator *da) const;
  virtual void show(std::ostream &out=std::cout) const;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_ATTRIBUTE_SINGLETON_SCORE_H */
