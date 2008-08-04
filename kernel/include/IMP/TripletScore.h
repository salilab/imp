/**
 *  \file TripletScore.h    \brief A Score on a triplet of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_TRIPLET_SCORE_H
#define __IMP_TRIPLET_SCORE_H

#include "IMP_config.h"
#include "base_types.h"
#include "RefCountedObject.h"
#include "Particle.h"
#include "DerivativeAccumulator.h"

namespace IMP
{
/**
   \ingroup restraint
   \addtogroup tripletscore Score functions on three particles
   Score functions to by applied to a triplet of particles. These can be
   used to make more flexible restraints.
 */

//! Abstract score function for a triplet of particles.
/** TripletScores should take a UnaryFunction as their first
    argument if such is needed.
*/
class IMPDLLEXPORT TripletScore : public RefCountedObject
{
public:
  TripletScore() {}
  virtual ~TripletScore() {}
  //! Compute the score for the triplet and the derivative if needed.
  virtual Float evaluate(Particle *a, Particle *b, Particle *c,
                         DerivativeAccumulator *da) const = 0;
  virtual void show(std::ostream &out=std::cout) const = 0;
};

IMP_OUTPUT_OPERATOR(TripletScore);

} // namespace IMP

#endif  /* __IMP_TRIPLET_SCORE_H */
