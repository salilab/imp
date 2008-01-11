/**
 *  \file AngleRestraint.h   \brief Angle restraint between three particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_ANGLE_RESTRAINT_H
#define __IMP_ANGLE_RESTRAINT_H

#include "../IMP_config.h"
#include "../UnaryFunction.h"
#include "../Restraint.h"


namespace IMP
{

//! Angle restraint between three particles
class IMPDLLEXPORT AngleRestraint : public Restraint
{
public:
  //! Create the angle restraint.
  /** \param[in] p1 Pointer to first particle in angle restraint.
      \param[in] p2 Pointer to second particle in angle restraint.
      \param[in] p3 Pointer to third particle in angle restraint.
      \param[in] score_func Scoring function for the restraint.
   */
  AngleRestraint(Particle* p1, Particle* p2, Particle* p3,
                 UnaryFunction* score_func);
  virtual ~AngleRestraint();

  IMP_RESTRAINT("0.1", "Ben Webb")

protected:
  //! scoring function for this restraint
  UnaryFunction* score_func_;
};

} // namespace IMP

#endif  /* __IMP_ANGLE_RESTRAINT_H */
