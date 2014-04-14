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
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/core/XYZ.h>
#include <IMP/algebra/eigen3/Eigen/StdVector>
#include <IMP/algebra/eigen3/Eigen/Dense>
#include <IMP/algebra/eigen3/Eigen/Geometry>

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

// Hide private nested classes from doxygen
#ifndef IMP_DOXYGEN
 private:
  class Referential {
   private:
    kernel::Model* m_;
    kernel::ParticleIndex pi_;
    IMP_Eigen::Vector3d centroid_;
    IMP_Eigen::Matrix3d base_;
    IMP_Eigen::Quaterniond q_;
    IMP_Eigen::Vector3d t_;

   public:
    Referential() {}  // undefined behaviour, don't use
    Referential(kernel::Model* m, kernel::ParticleIndex pi);
    // return properties of this rigid body
    IMP_Eigen::Vector3d get_centroid() const { return centroid_; }
    IMP_Eigen::Matrix3d get_base() const { return base_; }
    IMP_Eigen::Quaterniond get_rotation() const { return q_; }
    // project some properties on this or the global frame's coordinates
    IMP_Eigen::Vector3d get_local_coords(const IMP_Eigen::Vector3d
                                         & other) const;
    IMP_Eigen::Quaterniond get_local_rotation(const IMP_Eigen::Quaterniond
                                              & other) const;

   private:
    IMP_Eigen::Vector3d compute_centroid() const;
    IMP_Eigen::Matrix3d compute_base() const;
    IMP_Eigen::Quaterniond compute_quaternion() const;
    IMP_Eigen::Quaterniond pick_positive(const IMP_Eigen::Quaterniond& q) const;
  };

  class Transformer {
   private:
    kernel::Model* m_;
    Referential ref_;
    kernel::ParticleIndex target_;
    IMP_Eigen::Vector3d t_;
    IMP_Eigen::Quaterniond q_;
    bool moved_;

   public:
    // reorient other by t and q, expressed in this reference frame
    Transformer(kernel::Model* m, const Referential& ref,
                kernel::ParticleIndex other, const IMP_Eigen::Vector3d& t,
                const IMP_Eigen::Quaterniond& q)
        : m_(m), ref_(ref), target_(other), t_(t), q_(q), moved_(true) {
      transform();
    }
    // no-op
    Transformer() : moved_(false) {}
    void undo_transform();
    IMP_EIGEN_MAKE_ALIGNED_OPERATOR_NEW

   private:
    void transform();
  };

  struct Coord {
    std::vector<IMP_Eigen::Vector3d> coms;
    std::vector<IMP_Eigen::Quaterniond,
                IMP_Eigen::aligned_allocator<IMP_Eigen::Quaterniond> > quats;
    IMP_EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  };

  Coord get_coordinates_from_rbs() const;
  unsigned get_closest_entry_point(const Coord& x) const;
  Coord move_point(const Coord& x, const Coord& entry, const Coord& exit) const;
  double get_squared_distance(const Coord& x, const Coord& y) const;

 private:
  kernel::ParticleIndexes pis_;
  kernel::ParticleIndex ref_;
  double k_, move_probability_;
  std::vector<Transformer> last_transformations_;
  std::vector<Coord> entries_;
#endif
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_RIGID_BODY_TUNNELER_H */
