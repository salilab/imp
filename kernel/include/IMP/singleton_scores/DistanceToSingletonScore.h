/**
 *  \file DistanceToSingletonScore.h    
 *  \brief A Score on the distance to a fixed point.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_DISTANCE_TO_SINGLETON_SCORE_H
#define __IMP_DISTANCE_TO_SINGLETON_SCORE_H

#include "../SingletonScore.h"
#include "../Vector3D.h"
#include "../Pointer.h"
#include "../UnaryFunction.h"

namespace IMP
{

//! Apply a function to the distance to a fixed point.
/** \ingroup singleton
 */
class IMPDLLEXPORT DistanceToSingletonScore : public SingletonScore
{
  Pointer<UnaryFunction> f_;
  Vector3D pt_;
public:
  DistanceToSingletonScore(UnaryFunction *f, const Vector3D& pt);
  virtual ~DistanceToSingletonScore(){}
  virtual Float evaluate(Particle *a,
                         DerivativeAccumulator *da) const;
  virtual void show(std::ostream &out=std::cout) const;
};

} // namespace IMP

#endif  /* __IMP_DISTANCE_TO_SINGLETON_SCORE_H */
