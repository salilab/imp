/**
 *  \file AttributeSingletonScore.h
 *  \brief A score based on the unmodified value of an attribute.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMPCORE_ATTRIBUTE_SINGLETON_SCORE_H
#define __IMPCORE_ATTRIBUTE_SINGLETON_SCORE_H

#include "core_exports.h"

#include <IMP/SingletonScore.h>
#include <IMP/Pointer.h>
#include <IMP/UnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE

//! Apply a function to an attribute.
/** \ingroup singleton
 */
class IMPCOREEXPORT AttributeSingletonScore : public SingletonScore
{
  Pointer<UnaryFunction> f_;
  FloatKey k_;
public:
  AttributeSingletonScore(UnaryFunction *f, FloatKey k);
  virtual ~AttributeSingletonScore(){}
  virtual Float evaluate(Particle *a,
                         DerivativeAccumulator *da) const;
  virtual void show(std::ostream &out=std::cout) const;
};

IMPCORE_END_NAMESPACE

#endif  /* __IMPCORE_ATTRIBUTE_SINGLETON_SCORE_H */
