/**
 *  \file IMP/core/RigidBodyTunneler.h
 *  \brief A mover that transform a rigid body
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_RIGID_BODY_TUNNELER_H
#define IMPCORE_RIGID_BODY_TUNNELER_H

#include <IMP/core/core_config.h>
#include "MonteCarlo.h"
#include "MonteCarloMover.h"
#include <IMP/core/rigid_bodies.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/internal/tunneler_helpers.h>
#include <IMP/algebra/eigen3/Eigen/Dense>
#include <IMP/algebra/eigen3/Eigen/Geometry>
#include <iostream>

IMPCORE_BEGIN_NAMESPACE

//! Modify the transformation of a rigid body
/** Provided a number of entry points, this mover will propose random
 * translations of the rigid body from the closest entry point to a randomly
 * chosen other one. Moves that do not end up in the target exit area will be
 * dropped. This way of moving thus satisfies detailed balance. Entry points are
 * relative to the center of the provided reference rigid body.
 *
 * The procedure is as follows. Suppose we are at x.
 * pick closest entry point to x: k
 * do
 *    pick random other entry point if one is still available: l
 *    compute new position y by applying to x the transform that maps k to l
 * while (closest entry point to y is not l)
 * if (closest entry point to y is l)
 *    propose y as move
 * else
 *    do nothing
 *
 * \see RigidBodyMover
 * \see MonteCarlo
 */
class IMPCOREEXPORT RigidBodyTunneler : public MonteCarloMover {

 public:
  /** Constructor
   * \param m the model
   * \param pis the rigid bodies to move, in order
   * \param ref the rigidbody reference
   * \param k the weight when calculating distances
   *  The squared distance between two coordinates is
   *  \f$d^2_\text{eucl}(COM1,COM2) + k*d^2_\text{quat}(Q1,Q2)\f$
   *  If you want to weight rotation differencies at the same level as
   *  translational ones, pick \f$k \simeq d_\text{max}/\pi\f$ where
   *  \f$d_\text{max}\f$ is the largest translational difference to be expected.
   * \param move_probability the prior probability to actually
   * move somewhere else
   */
  RigidBodyTunneler(kernel::Model* m, kernel::ParticleIndexes pis,
                    kernel::ParticleIndex ref, double k,
                    double move_probability = 1.);

  //! add entry point
  /** each entry point is a list of coordinates that matches the number of rigid
   * bodies in pis (see constructor)
   * If there are N rigid bodies in pis, then an entry point is
   * - 3N floats, representing the x,y,z coordinates of the
   *   centroid of each of the N rigid bodies
   * - 4N floats, corresponding to the N rotational quaternions of the rbs
   * Coordinates are all relative to the reference, i.e. for which the centroid
   * is at the origin and the rotation is identity.
   * TODO: If an entry point should be indifferent of one rigid body, one
   * centroid or one quaternion, the corresponding floats should be set to zero.
   */
  void add_entry_point(Floats fl);

  /// Statistics
  void reset_stats();
  unsigned get_number_of_rejected_moves() const { return num_rejected_; }
  unsigned get_number_of_proposed_moves() const { return num_proposed_; }
  unsigned get_number_of_impossible_moves() const { return num_impossible_; }
  unsigned get_number_of_calls() const { return num_calls_; }

  //! returns center of mass and quaternion of rotation wrt ref
  static Floats get_reduced_coordinates(kernel::Model* m,
                                        kernel::ParticleIndex target,
                                        kernel::ParticleIndex ref);

  //! returns center of mass and quaternion of rotation of pi
  static Floats get_reduced_coordinates(kernel::Model* m,
                                        kernel::ParticleIndex pi);

  /// sets rigid body coordinates in the reference frame of ref
  // void set_reduced_coordinates(algebra::VectorD<3> com, algebra::VectorD<4>
  // q);

 protected:
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RigidBodyTunneler);

 private:
  internal::Coord get_coordinates_from_rbs() const;
  unsigned get_closest_entry_point(const internal::Coord& x) const;
  double get_squared_distance(const internal::Coord& x, const internal::Coord& y) const;

 private:
  kernel::ParticleIndexes pis_;
  kernel::ParticleIndex ref_;
  double k_, move_probability_;
  unsigned num_calls_, num_proposed_, num_rejected_, num_impossible_;
  std::vector<internal::Transformer> last_transformations_;
  std::vector<internal::Coord> entries_;
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_RIGID_BODY_TUNNELER_H */
