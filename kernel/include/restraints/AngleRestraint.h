/**
 *  \file AngleRestraint.h   \brief Angle restraint between three particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_ANGLE_RESTRAINT_H
#define __IMP_ANGLE_RESTRAINT_H

#include "../IMP_config.h"
#include "../Restraint.h"
#include "../internal/kernel_version_info.h"
#include "../Pointer.h"
#include "../triplet_scores/AngleTripletScore.h"

IMP_BEGIN_NAMESPACE

class UnaryFunction;

//! Angle restraint between three particles
class IMPDLLEXPORT AngleRestraint : public Restraint
{
public:
  //! Create the angle restraint.
  /** \param[in] score_func Scoring function for the restraint.
      \param[in] p1 Pointer to first particle in angle restraint.
      \param[in] p2 Pointer to second particle in angle restraint.
      \param[in] p3 Pointer to third particle in angle restraint.
   */
  AngleRestraint(UnaryFunction* score_func,
                 Particle* p1, Particle* p2, Particle* p3);
  virtual ~AngleRestraint(){}

  IMP_RESTRAINT(internal::kernel_version_info)

protected:
    Pointer<AngleTripletScore> sf_;
};

IMP_END_NAMESPACE

#endif  /* __IMP_ANGLE_RESTRAINT_H */
