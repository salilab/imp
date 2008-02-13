/**
 *  \file SingletonScore.h    \brief A Score on a single particle.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_SINGLETON_SCORE_H
#define __IMP_SINGLETON_SCORE_H

#include "IMP_config.h"
#include "base_types.h"
#include "Object.h"
#include "DerivativeAccumulator.h"

namespace IMP
{

class Particle;

/** \ingroup restraint
    \addtogroup singleton Score functions on one particle
    Score functions to be applied to a single particle. These can be
    used to make more flexible restraints.
 */

//! Abstract score function for a single particle.
class IMPDLLEXPORT SingletonScore : public Object
{
public:
  SingletonScore() {}
  virtual ~SingletonScore() {}
  //! Compute the score for the particle and the derivative if needed.
  virtual Float evaluate(Particle *a,
                         DerivativeAccumulator *da) = 0;
  virtual void show(std::ostream &out=std::cout) const = 0;
};

} // namespace IMP

#endif  /* __IMP_SINGLETON_SCORE_H */
