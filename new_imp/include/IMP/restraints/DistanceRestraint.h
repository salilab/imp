/**
 *  \file DistanceRestraint.h   \brief Distance restraint between two particles.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_DISTANCE_RESTRAINT_H
#define __IMP_DISTANCE_RESTRAINT_H

#include <vector>
#include <iostream>

#include "../IMP_config.h"
#include "../ModelData.h"
#include "../ScoreFunc.h"
#include "../Restraint.h"


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

  DistanceRestraint(Model* model, Particle* p1, Particle* p2,
                    ScoreFunc* score_func);
  virtual ~DistanceRestraint();

  IMP_RESTRAINT("0.5", "Daniel Russel")

protected:
  //! Do set up for the distance restraint constructors.
  /** \param[in] model Pointer to the model.
      \param[in] p1 Pointer to first particle in distance restraint.
      \param[in] p2 Pointer to second particle in distance restraint.
      \param[in] score_func Scoring function for the restraint.
   */
  void set_up(Model* model, Particle* p1, Particle* p2,
              ScoreFunc* score_func);

  //! scoring function for this restraint
  ScoreFunc* score_func_;
};

} // namespace IMP

#endif  /* __IMP_DISTANCE_RESTRAINT_H */
