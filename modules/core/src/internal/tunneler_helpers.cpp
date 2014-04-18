/**
 *  \file tunneler_helpers.cpp
 *  \brief helper classes for RigidBodyTunneler
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/internal/tunneler_helpers.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

Referential::Referential(kernel::Model* m,
                                            kernel::ParticleIndex pi)
    : m_(m), pi_(pi), centroid_(compute_centroid()), base_(compute_base()),
      q_(compute_quaternion()) {}

IMP_Eigen::Vector3d Referential::compute_centroid() const {
  // get rigid body member coordinates
  RigidBody d(m_, pi_);
  kernel::ParticleIndexes pis(d.get_member_particle_indexes());
  IMP_Eigen::Matrix<double, IMP_Eigen::Dynamic, 3> coords(pis.size(), 3);
  for (unsigned i = 0; i < pis.size(); i++) {
    XYZ xyz(m_, pis[i]);
    coords(i, 0) = xyz.get_x();
    coords(i, 1) = xyz.get_y();
    coords(i, 2) = xyz.get_z();
  }
  return coords.colwise().mean();
}

IMP_Eigen::Matrix3d Referential::compute_base() const {
  RigidBody d(m_, pi_);
  kernel::ParticleIndexes pis(d.get_member_particle_indexes());
  if (pis.size() < 3)
    IMP_THROW("rigid body must contain at least 3 xyzs", ModelException);
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

IMP_Eigen::Quaterniond Referential::compute_quaternion()
    const {
  return pick_positive(IMP_Eigen::Quaterniond(base_));
}

IMP_Eigen::Vector3d Referential::get_local_coords(
    const IMP_Eigen::Vector3d& other) const {
  return q_.conjugate() * (other - centroid_);
}

IMP_Eigen::Quaterniond Referential::get_local_rotation(
    const IMP_Eigen::Quaterniond& other) const {
  return pick_positive(q_.conjugate() * other); // * q_);
}

IMP_Eigen::Quaterniond pick_positive(const IMP_Eigen::Quaterniond& other) {
  if (other.w() < 0) {
    return IMP_Eigen::Quaterniond(-other.w(), -other.x(), -other.y(),
                                  -other.z());
  } else {
    return other;
  }
}

void Transformer::transform() {
  // convert translation and rotation from local to global coords
  IMP_Eigen::Quaterniond refq = ref_.get_rotation();
  IMP_Eigen::Vector3d global_t(refq * t_);
  IMP_Eigen::Quaterniond global_q(pick_positive(refq * q_ * refq.conjugate()));
  // get rb centroid as a translation
  IMP_Eigen::Vector3d centroid(Referential(m_, target_).get_centroid());
  // transform each rigid member
  RigidBody d(m_, target_);
  kernel::ParticleIndexes pis(d.get_member_particle_indexes());
  for (unsigned i = 0; i < pis.size(); i++) {
    XYZ xyz(m_, pis[i]);
    IMP_Eigen::Vector3d coords;
    coords << xyz.get_x(), xyz.get_y(), xyz.get_z();
    IMP_Eigen::Vector3d newcoords = global_q*(coords-centroid)
                                    +centroid+global_t;
    xyz.set_x(newcoords(0));
    xyz.set_y(newcoords(1));
    xyz.set_z(newcoords(2));
  }
  // update rigid body
  d.set_reference_frame_from_members(pis);
}

bool Transformer::undo_transform() {
  if (moved_) {
    t_ = -t_;
    q_ = q_.conjugate();
    transform();
    moved_ = false;
    return true;
  }
  return false;
}

std::ostream& operator<<(std::ostream& out, const Coord& c) {
    for (Coord::com_type::const_iterator it = c.coms.begin();
            it != c.coms.end(); ++it)
        out << " xyz= " << it->transpose() << " ";
    for (Coord::quats_type::const_iterator it = c.quats.begin();
            it != c.quats.end(); ++it)
        out << " w=" << it->w()
            << " x=" << it->x() 
            << " y=" << it->y()
            << " z=" << it->z() << " ";
    out << std::endl;
    return out;
}

IMPCORE_END_INTERNAL_NAMESPACE
