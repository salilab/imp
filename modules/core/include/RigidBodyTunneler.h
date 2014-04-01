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
#include <IMP/algebra/eigen3/Eigen/Dense>
#include <IMP/algebra/eigen3/Eigen/Geometry>

IMPCORE_BEGIN_NAMESPACE

//! Modify the transformation of a rigid body
/** Provided a number of entry points in cartesian space, this mover will
 * propose random translations of the rigid body from the closest entry point to
 * a randomly chosen other one. This way of moving thus satisfies detailed
 * balance. Entry points are relative to the center of the provided reference
 * rigid body.
 *
 * \see RigidBodyMover
 * \see MonteCarlo
 */
class IMPCOREEXPORT RigidBodyTunneler : public MonteCarloMover {

 public:
  /** Constructor
   * \param m the model
   * \param pi the rigidbody to move
   * \param ref the rigidbody reference
   * \param move_probability the prior probability to actually
   * move somewhere else
   */
  RigidBodyTunneler(kernel::Model* m, kernel::ParticleIndex pi,
                    kernel::ParticleIndex ref, double move_probability = 1.);

  /// add entry point in cartesian space
  void add_entry_point(IMP_Eigen::Vector3d pt) { entries_.push_back(pt); }

  /// returns center of mass coordinates and quaternion of rotation wrt ref
  Floats get_reduced_coordinates() const;

  /// returns center of mass coordinates and quaternion of rotation of pi
  static Floats get_reduced_coordinates(kernel::Model*m,
          kernel::ParticleIndex pi);

  /// sets rigid body coordinates in the reference frame of ref
  //void set_reduced_coordinates(algebra::VectorD<3> com, algebra::VectorD<4> q);

 protected:
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RigidBodyTunneler);

 private:
  class Referential {
   private:
    kernel::Model *m_;
    kernel::ParticleIndex pi_;
    IMP_Eigen::Vector3d centroid_;
    IMP_Eigen::Matrix3d base_;
    IMP_Eigen::Quaterniond q_;
    IMP_Eigen::Vector3d t_;

   public:
    Referential() {} //undefined behaviour, don't use
    Referential(kernel::Model *m, kernel::ParticleIndex pi);
    // return properties of this rigid body
    IMP_Eigen::Vector3d get_centroid() const { return centroid_; }
    IMP_Eigen::Matrix3d get_base() const { return base_; }
    IMP_Eigen::Quaterniond get_rotation() const { return q_; }
    // project some properties on this or the global frame's coordinates
    IMP_Eigen::Vector3d get_local_coords(const IMP_Eigen::Vector3d
                                         & other) const;
    IMP_Eigen::Quaterniond get_local_rotation(const IMP_Eigen::Quaterniond
                                              & other) const;
    IMP_Eigen::Vector3d get_global_coords(const IMP_Eigen::Vector3d
                                         & other) const;

   private:
    IMP_Eigen::Vector3d compute_centroid() const;
    IMP_Eigen::Matrix3d compute_base() const;
    IMP_Eigen::Quaterniond compute_quaternion() const;
    IMP_Eigen::Quaterniond pick_positive(const IMP_Eigen::Quaterniond& q) const;
  };

  class Translater {
   private:
    kernel::Model *m_;
    Referential ref_;
    kernel::ParticleIndex target_;
    IMP_Eigen::Vector3d t_;
    bool moved_;

   public:
    // translate other by t in this reference frame
    Translater(kernel::Model* m, const Referential& ref,
               kernel::ParticleIndex other, const IMP_Eigen::Vector3d& t)
        : m_(m), ref_(ref), target_(other), t_(t), moved_(true) {
      translate();
    }
    // no-op
    Translater() : moved_(false) {}
    void undo_translate();
   private:
    void translate();
  };

 private:
  kernel::ParticleIndex pi_, ref_;
  double move_probability_;
  Translater last_translation_;
  std::vector<IMP_Eigen::Vector3d> entries_; //Vector3d not vectorizable

};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_RIGID_BODY_TUNNELER_H */
