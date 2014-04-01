/**
 *  \file RigidBodyTunneler.cpp
 *  \brief A mover that transforms a rigid body
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core/RigidBodyTunneler.h>
#include <IMP/base/random.h>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_01.hpp>
#include <IMP/core/XYZ.h>
#include <IMP/core/rigid_bodies.h>

IMPCORE_BEGIN_NAMESPACE

RigidBodyTunneler::Referential::Referential(kernel::Model* m,
                                            kernel::ParticleIndex pi)
    : m_(m), pi_(pi), centroid_(compute_centroid()), base_(compute_base()),
      q_(compute_quaternion()) {}

IMP_Eigen::Vector3d RigidBodyTunneler::Referential::compute_centroid() const {
  // get rigid body member coordinates
  RigidBody d(m_, pi_);
  kernel::ParticleIndexes pis(d.get_member_particle_indexes());
  IMP_Eigen::Matrix<double, IMP_Eigen::Dynamic, 3> coords(pis.size(),3);
  for (unsigned i = 0; i < pis.size(); i++) {
    XYZ xyz(m_, pis[i]);
    coords(i, 0) = xyz.get_x();
    coords(i, 1) = xyz.get_y();
    coords(i, 2) = xyz.get_z();
  }
  return coords.colwise().mean();
}

IMP_Eigen::Matrix3d RigidBodyTunneler::Referential::compute_base() const {
  RigidBody d(m_, pi_);
  kernel::ParticleIndexes pis(d.get_member_particle_indexes());
  if (pis.size() < 3) IMP_THROW("rigid body must contain at least 3 xyzs",
                                ModelException);
  XYZ o(m_, pis[0]), x(m_, pis[1]), y(m_, pis[2]);
  IMP_Eigen::Vector3d vo, e1, e2;
  vo << o.get_x(), o.get_y(), o.get_z();
  e1 << x.get_x(), x.get_y(), x.get_z();
  e1 = e1 - vo;
  e1.normalize();
  e2 << y.get_x(), y.get_y(), y.get_z();
  e2 = e2 - vo;
  e2 = e2 - e2.dot(e1) * e1;
  e2.normalize();
  IMP_Eigen::Matrix3d retmat;
  retmat << e1, e2, e1.cross(e2);
  return retmat;
}

IMP_Eigen::Quaterniond RigidBodyTunneler::Referential::compute_quaternion()
    const {
  return pick_positive(IMP_Eigen::Quaterniond(base_));
}

IMP_Eigen::Vector3d RigidBodyTunneler::Referential::get_local_coords(
    const IMP_Eigen::Vector3d& other) const {
  return q_.conjugate() * (other - centroid_);
}

IMP_Eigen::Quaterniond RigidBodyTunneler::Referential::get_local_rotation(
    const IMP_Eigen::Quaterniond& other) const {
  return pick_positive(q_.inverse() * other);
}

IMP_Eigen::Quaterniond RigidBodyTunneler::Referential::pick_positive(
    const IMP_Eigen::Quaterniond& other) const {
  if (other.w() < 0) {
    return IMP_Eigen::Quaterniond(-other.w(),-other.x(),-other.y(),-other.z());
  } else {
    return other;
  }
}

void RigidBodyTunneler::Translater::translate() {
  // convert translation vector from local to global coords
  IMP_Eigen::Vector3d global_t(ref_.get_rotation()*t_);
  // translate each rigid member
  RigidBody d(m_, target_);
  kernel::ParticleIndexes pis(d.get_member_particle_indexes());
  for (unsigned i = 0; i < pis.size(); i++) {
    XYZ xyz(m_, pis[i]);
    IMP_Eigen::Vector3d coords;
    coords << xyz.get_x(), xyz.get_y(), xyz.get_z();
    IMP_Eigen::Vector3d newcoords(coords + global_t);
    xyz.set_x(newcoords(0));
    xyz.set_y(newcoords(1));
    xyz.set_z(newcoords(2));
  }
  //update rigid body
  d.set_reference_frame_from_members(pis);
}

void RigidBodyTunneler::Translater::undo_translate() {
  if (moved_) {
    t_ = -t_;
    translate();
    moved_ = false;
  }
}

RigidBodyTunneler::RigidBodyTunneler(kernel::Model* m, kernel::ParticleIndex pi,
                                     kernel::ParticleIndex ref,
                                     double move_probability)
    : MonteCarloMover(m, m->get_particle(pi)->get_name() + " tunneler"),
      pi_(pi), ref_(ref), move_probability_(move_probability) {
  IMP_USAGE_CHECK(RigidBody(m, pi).get_coordinates_are_optimized(),
                  "Rigid body passed to RigidBodyTunneler"
                  << " must be set to be optimized. particle: "
                  << m->get_particle_name(pi));
  IMP_USAGE_CHECK(move_probability_ >= 0 && move_probability <= 1,
                  "Probability must be 0<= p <= 1");
}

MonteCarloMoverResult RigidBodyTunneler::do_propose() {
  IMP_OBJECT_LOG;
  // check whether mover is usable
  if (entries_.size() < 2)
    IMP_THROW("You must add at least two entry points for this "
              << "mover to be usable.", ModelException);
  IMP_USAGE_CHECK(
      RigidBody d(get_model(), pi_).get_coordinates_are_optimized(),
      "Rigid body passed to RigidBodyTunneler"
      << " must be set to be optimized. particle: " << d->get_name());
  // get current reference frame of rb
  Referential target(get_model(), pi_), referential(get_model(), ref_);
  // see if we are to do the move
  ::boost::uniform_01<double> rand01;
  if (rand01(base::random_number_generator) <= move_probability_) {
    // get centroid of target in frame of ref
    IMP_Eigen::Vector3d com(
        referential.get_local_coords(target.get_centroid()));
    // compute the closest entry point
    unsigned closest = 0;
    double mindistsq;
    for (unsigned i = 0; i < entries_.size(); i++) {
      const double distsq = (entries_[i] - com).squaredNorm();
      if (i == 0 || mindistsq > distsq) {
        mindistsq = distsq;
        closest = i;
      }
    }
    // pick another entry point at random
    ::boost::uniform_int<unsigned> randint(0, entries_.size() - 1);
    unsigned distant;
    do {
      distant = randint(base::random_number_generator);
    } while (distant == closest);
    // propose move to it
    IMP_Eigen::Vector3d t(entries_[distant] - entries_[closest]);
    last_translation_ = Translater(get_model(), referential, pi_, t);
    IMP_LOG_VERBOSE("proposed move " << t.transpose() << " from entry point "
                                     << closest << " to " << distant
                                     << std::endl);
  }
  return MonteCarloMoverResult(kernel::ParticleIndexes(1, pi_), 1.0);
}

void RigidBodyTunneler::do_reject() { last_translation_.undo_translate(); }

Floats RigidBodyTunneler::get_reduced_coordinates(kernel::Model *m,
        kernel::ParticleIndex t, kernel::ParticleIndex r) {
  Referential target(m, t), referential(m, r);
  IMP_Eigen::Vector3d com(
        referential.get_local_coords(target.get_centroid()));
  IMP_Eigen::Quaterniond rot(
        referential.get_local_rotation(target.get_rotation()));
  Floats retval;
  retval.push_back(com(0));
  retval.push_back(com(1));
  retval.push_back(com(2));
  retval.push_back(rot.w());
  retval.push_back(rot.x());
  retval.push_back(rot.y());
  retval.push_back(rot.z());
  return retval;
}

Floats RigidBodyTunneler::get_reduced_coordinates(kernel::Model *m,
        kernel::ParticleIndex pi) {
  Referential target(m, pi);
  IMP_Eigen::Vector3d com(target.get_centroid());
  IMP_Eigen::Quaterniond rot(target.get_rotation());
  Floats retval;
  retval.push_back(com(0));
  retval.push_back(com(1));
  retval.push_back(com(2));
  retval.push_back(rot.w());
  retval.push_back(rot.x());
  retval.push_back(rot.y());
  retval.push_back(rot.z());
  return retval;
}

kernel::ModelObjectsTemp RigidBodyTunneler::do_get_inputs() const {
  return kernel::ModelObjectsTemp(1, get_model()->get_particle(pi_));
}

IMPCORE_END_NAMESPACE
