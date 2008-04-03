/**
 *  \file GravityCenterScoreState.h
 *  \brief Set particle to match the gravity center of one or more particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_GRAVITY_CENTER_SCORE_STATE_H
#define __IMP_GRAVITY_CENTER_SCORE_STATE_H

#include "../ScoreState.h"
#include "../Particle.h"
#include "../DerivativeAccumulator.h"

#include <vector>

namespace IMP
{

//! Set particle position to match the gravity center of one or more particles.
/** The center particle will be assigned an XYZ position corresponding to the
    gravity center of the point particles, optionally weighted by a float
    attribute of these particles (e.g. mass). Restraints can then be placed on
    the center particle, and any forces on it will be back-transformed onto the
    points. Note that the xyz attributes of the center particle are not
    themselves optimizable, since they are derived from the original points.
 */
class IMPDLLEXPORT GravityCenterScoreState: public ScoreState
{
public:
  //! Constructor.
  /** \param[in] center Particle to be constrained to the gravity center.
      \param[in] weightkey Float attribute to weight coordinates (if not
                           specified, or given as FloatKey(), no weighting).
      \param[in] ps Original particles used to calculate the center.
   */
  GravityCenterScoreState(Particle *center, FloatKey weightkey=FloatKey(),
                          const Particles &ps= Particles()) 
      : center_(center), weightkey_(weightkey) {
    add_particles(ps);
  }
  virtual ~GravityCenterScoreState() {}

  //! Set the position of the center particle from the original points.
  void set_position();

  void update() {
    set_position();
  }

  void after_evaluate(DerivativeAccumulator *accpt) {
    if (accpt) {
      transform_derivatives(accpt);
    }
  }

protected:
  //! Back-transform any forces on the center particle to the original points.
  void transform_derivatives(DerivativeAccumulator *accpt);

  Particle *center_;
  Particles ps_;
  FloatKey weightkey_;

  IMP_LIST(public, Particle, particle, Particle*);
};

} // namespace IMP

#endif  /* __IMP_GRAVITY_CENTER_SCORE_STATE_H */
