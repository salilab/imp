/**
 *  \file TorusRestraint.h   \brief Score based on distance from torus interior
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_TORUS_RESTRAINT_H
#define __IMP_TORUS_RESTRAINT_H

#include <list>

#include "../IMP_config.h"
#include "../ScoreFuncParams.h"
#include "../Restraint.h"

namespace IMP
{

//! Restrict particle position to interior of a torus
class IMPDLLEXPORT TorusRestraint : public Restraint
{
public:
  /** \param[in] p1 Pointer to particle of the restraint.
      \param[in] main_radius The main radius from the origin to the midline
                             of the tube.
      \param[in] tube_radius The tube radius is min distance from the tube
                             midline to the tube surface.
      \param[in] score_func  Scoring function for the torus feature (distance
                             of the particle from the torus surface)
   */
  TorusRestraint(Particle* p1, const Float main_radius,
                 const Float tube_radius, UnaryFunctor* score_func);
  virtual ~TorusRestraint();

  IMP_RESTRAINT("0.6", "Ben Webb")
protected:
  //! variables used to determine the distance
  FloatKey x_, y_, z_;
  //! main radius of the torus
  Float main_radius_;
  //! radius of the torus tube
  Float tube_radius_;
  //! math form for this restraint (typically one of the harmonics)
  UnaryFunctor* score_func_;
};

} // namespace IMP

#endif /* __IMP_TORUS_RESTRAINT_H */
