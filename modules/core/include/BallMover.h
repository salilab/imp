/**
 *  \file IMP/core/BallMover.h
 *  \brief A modifier which variables within a ball.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_BALL_MOVER_H
#define IMPCORE_BALL_MOVER_H

#include <IMP/core/core_config.h>
#include "MonteCarloMover.h"

IMPCORE_BEGIN_NAMESPACE

//! Modify a set of continuous variables by perturbing them within a ball.
/** The variables are perturbed within a ball of the
    given radius.
    \see MonteCarlo
 */
class IMPCOREEXPORT BallMover: public MonteCarloMover
{
  ParticleIndexes pis_;
  FloatKeys keys_;
  double radius_;
  algebra::VectorKDs originals_;

  void initialize(ParticleIndexes pis,
                  FloatKeys keys,
                  double radius);
public:
  BallMover(Model *m, ParticleIndex pi,
            const FloatKeys &vars, double radius);
  //! Move the x,y,z coordinates
  BallMover(Model *m, ParticleIndex pi, double radius);

#ifndef IMP_DOXYGEN
  /** The attributes are perturbed within a ball whose dimensionality is
      given by the number of attributes and radius by the given value.
      \param[in] sc The set of particles to perturb.
      \param[in] vars The variables to use (normally the keys for x,y,z)
      \param[in] radius The radius deviation to use.
   */
  BallMover(const ParticlesTemp &sc, const FloatKeys &vars,
            Float radius);

  /** The x,y,z coordinates are perturbed within a ball.
      \param[in] sc The set of particles to perturb.
      \param[in] radius The radius deviation to use.
   */
  BallMover(const ParticlesTemp &sc,
            Float radius);
#endif

  void set_radius(Float radius) {
    IMP_USAGE_CHECK(radius > 0, "The radius must be positive");
    radius_=radius;
  }

  Float get_radius() const {
    return radius_;
  }

 protected:
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(BallMover);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BALL_MOVER_H */
