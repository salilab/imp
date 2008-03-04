/**
 *  \file AttributeSingletonScore.h    
 *  \brief A score based on the unmodified value of an attribute.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_ATTRIBUTE_SINGLETON_SCORE_H
#define __IMP_ATTRIBUTE_SINGLETON_SCORE_H

#include "../SingletonScore.h"
#include "../internal/ObjectPointer.h"

namespace IMP
{

class UnaryFunction;

//! Apply a function to an attribute.
/** \ingroup singleton
 */
class IMPDLLEXPORT AttributeSingletonScore : public SingletonScore
{
  internal::ObjectPointer<UnaryFunction, true> f_;
  FloatKey k_;
public:
  AttributeSingletonScore(UnaryFunction *f, FloatKey k);
  virtual ~AttributeSingletonScore(){}
  virtual Float evaluate(Particle *a,
                         DerivativeAccumulator *da);
  virtual void show(std::ostream &out=std::cout) const;
};

} // namespace IMP

#endif  /* __IMP_ATTRIBUTE_SINGLETON_SCORE_H */
