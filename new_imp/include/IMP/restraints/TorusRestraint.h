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
#include "../Restraint.h"

namespace IMP
{

//! Restrict particle position to interior of a torus
class IMPDLLEXPORT TorusRestraint : public Restraint
{
public:
  TorusRestraint(Model& model, Particle* p1, const Float main_radius,
                 const Float tube_radius,
                 BasicScoreFuncParams* score_func_params);
  virtual ~TorusRestraint();

  IMP_RESTRAINT("0.5", "Daniel Russel")
protected:
  //! variables used to determine the distance
  FloatKey x_, y_, z_;
  //! main radius of the torus
  Float main_radius_;
  //! radius of the torus tube
  Float tube_radius_;
  //! math form for this restraint (typically one of the harmonics)
  ScoreFunc* score_func_;
};

} // namespace IMP

#endif /* __IMP_TORUS_RESTRAINT_H */
