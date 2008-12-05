/**
 *  \file TripletScore.h    \brief A Score on a triplet of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMP_TRIPLET_SCORE_H
#define IMP_TRIPLET_SCORE_H

#include "IMP_config.h"
#include "base_types.h"
#include "RefCountedObject.h"
#include "Particle.h"
#include "DerivativeAccumulator.h"

IMP_BEGIN_NAMESPACE

//! Abstract score function for a triplet of particles.
/** TripletScores should take a UnaryFunction as their first
    argument if such is needed.
*/
class IMPDLLEXPORT TripletScore : public RefCountedObject
{
public:
  TripletScore() {}
  //! Compute the score for the triplet and the derivative if needed.
  virtual Float evaluate(Particle *a, Particle *b, Particle *c,
                         DerivativeAccumulator *da) const = 0;
  //! Print information about the TripletScore to a stream.
  /** Should end in a newline.
   */
  virtual void show(std::ostream &out=std::cout) const = 0;

  IMP_REF_COUNTED_DESTRUCTOR(TripletScore)
};

IMP_OUTPUT_OPERATOR(TripletScore);

IMP_END_NAMESPACE

#endif  /* IMP_TRIPLET_SCORE_H */
