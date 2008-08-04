/**
 *  \file AttributeSingletonScore.h    
 *  \brief A score based on the unmodified value of an attribute.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_ATTRIBUTE_SINGLETON_SCORE_H
#define __IMP_ATTRIBUTE_SINGLETON_SCORE_H

#include "../SingletonScore.h"
#include "../Pointer.h"
#include "../UnaryFunction.h"

namespace IMP
{

//! Apply a function to an attribute.
/** \ingroup singleton
 */
class IMPDLLEXPORT AttributeSingletonScore : public SingletonScore
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

} // namespace IMP

#endif  /* __IMP_ATTRIBUTE_SINGLETON_SCORE_H */
