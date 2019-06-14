/**
 *  \file IMP/core/BallMover.h
 *  \brief A modifier which variables within a ball.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_BALL_MOVER_H
#define IMPCORE_BALL_MOVER_H

#include <IMP/core/core_config.h>
#include <IMP/base_types.h>
#include <IMP/exception.h>
#include "MonteCarloMover.h"

IMPCORE_BEGIN_NAMESPACE

//! Move continuous particle variables by perturbing them within a ball.
/** The variables are perturbed within a ball of the given radius.
    \see MonteCarlo
 */
class IMPCOREEXPORT BallMover : public MonteCarloMover {
  ParticleIndexes pis_;
  FloatKeys keys_;
  double radius_;
  algebra::VectorKDs originals_;

  void initialize(ParticleIndexes pis, FloatKeys keys, double radius);

 public:
  //! Move specified variables of particle pi within a ball of specified radius
  /** Construct a mover that in each move, perturbs the specified
      variables (attributes) of particle pi in model m, within a ball
      of specified radius, whose dimensionality is the total number of
      attributes.
  */
  BallMover(Model *m, ParticleIndex pi, const FloatKeys &vars,
            double radius);

  //! Move the x,y,z coordinates of pi within a ball of specified radius
  BallMover(Model *m, ParticleIndex pi, double radius);

  //! Move specified variables of particles within a ball of specified radius
  /** \param[in] pis The set of particles to perturb.
      \param[in] vars The variables to use (normally the keys for x,y,z)
      \param[in] radius The radius deviation to use.
   */
  BallMover(Model *m, const ParticleIndexes &pis, const FloatKeys &vars,
            Float radius);

  //! Move the x,y,z coordinates of pis within a ball of specified radius
  /** The x,y,z coordinates of each particle are perturbed within a ball.
      \param[in] pis The set of particles to perturb.
      \param[in] radius The radius deviation to use.
   */
  BallMover(Model *m, const ParticleIndexes &pis, Float radius);

  void set_radius(Float radius) {
    IMP_ALWAYS_CHECK(radius > 0, "The radius must be positive",
                     IMP::ValueException);
    radius_ = radius;
  }

  Float get_radius() const { return radius_; }

 protected:
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;

  //! Move particle attributes within a ball, as specified in constructor
  virtual MonteCarloMoverResult do_propose() IMP_OVERRIDE;

  //! Restore original attributes from before do_propose()
  virtual void do_reject() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(BallMover);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_BALL_MOVER_H */
