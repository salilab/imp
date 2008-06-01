/**
 *  \file DistanceRestraint.h   \brief Distance restraint between two particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_DISTANCE_RESTRAINT_H
#define __IMP_DISTANCE_RESTRAINT_H

#include "../IMP_config.h"
#include "../pair_scores/DistancePairScore.h"
#include "../Restraint.h"
#include "../internal/kernel_version_info.h"

#include <iostream>

namespace IMP
{

//! Distance restraint between two particles
/**
   \note If the particles are closer than a certain distance, then
   the contributions to the derivatives are set to 0.
 */
class IMPDLLEXPORT DistanceRestraint : public Restraint
{
public:
  //! Create the distance restraint.
  /** \param[in] score_func Scoring function for the restraint.
      \param[in] p1 Pointer to first particle in distance restraint.
      \param[in] p2 Pointer to second particle in distance restraint.
   */
  DistanceRestraint(UnaryFunction* score_func,
                    Particle* p1, Particle* p2);
  virtual ~DistanceRestraint() {}

  IMP_RESTRAINT(internal::kernel_version_info)

protected:
  //! scoring function for this restraint
  DistancePairScore dp_;
};

} // namespace IMP

#endif  /* __IMP_DISTANCE_RESTRAINT_H */
