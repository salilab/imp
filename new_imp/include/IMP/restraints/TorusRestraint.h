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
#include "Restraint.h"

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

  //! Calculate the score for this restraint for the current model state.
  /** \param[in] accum If not NULL, use this object to accumulate partial first
                       derivatives.
      \return Current score.
   */
  virtual Float evaluate(DerivativeAccumulator *accum);

  //! Show the current restraint.
  /** \param[in] out Stream to send restraint description to.
   */
  virtual void show(std::ostream& out = std::cout) const;

  virtual std::string version(void) const {
    return "0.5.0";
  }
  virtual std::string last_modified_by(void) const {
    return "Bret Peterson";
  }

protected:
  //! variables used to determine the distance
  FloatIndex x1_, y1_, z1_;
  //! main radius of the torus
  Float main_radius_;
  //! radius of the torus tube
  Float tube_radius_;
  //! math form for this restraint (typically one of the harmonics)
  ScoreFunc* score_func_;
};

} // namespace IMP

#endif /* __IMP_TORUS_RESTRAINT_H */
