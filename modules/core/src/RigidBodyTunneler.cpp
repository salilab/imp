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
  IMP_Eigen::Matrix<double, IMP_Eigen::Dynamic, 3> coords(pis.size(), 3);
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
    return IMP_Eigen::Quaterniond(-other.w(), -other.x(), -other.y(),
                                  -other.z());
  } else {
    return other;
  }
}

void RigidBodyTunneler::Transformer::transform() {
  // convert translation and rotation from local to global coords
  IMP_Eigen::Translation3d global_t(ref_.get_rotation() * t_);
  IMP_Eigen::Quaterniond global_q(q_ * ref_.get_rotation());
  // get rb centroid as a translation
  IMP_Eigen::Translation3d centroid(Referential(m_, target_).get_centroid());
  // build transformation from it
  // first remove center of mass, then rotate, finally translate by the rest
  IMP_Eigen::Isometry3d t;
  t = centroid.inverse() * global_q * centroid * global_t;
  // transform each rigid member
  RigidBody d(m_, target_);
  kernel::ParticleIndexes pis(d.get_member_particle_indexes());
  for (unsigned i = 0; i < pis.size(); i++) {
    XYZ xyz(m_, pis[i]);
    IMP_Eigen::Vector3d coords;
    coords << xyz.get_x(), xyz.get_y(), xyz.get_z();
    IMP_Eigen::Vector3d newcoords(t * coords);
    xyz.set_x(newcoords(0));
    xyz.set_y(newcoords(1));
    xyz.set_z(newcoords(2));
  }
  // update rigid body
  d.set_reference_frame_from_members(pis);
}

void RigidBodyTunneler::Transformer::undo_transform() {
  if (moved_) {
    t_ = -t_;
    q_ = q_.conjugate();
    transform();
    moved_ = false;
  }
}

RigidBodyTunneler::RigidBodyTunneler(kernel::Model* m,
                                     kernel::ParticleIndexes pis,
                                     kernel::ParticleIndex ref, double k,
                                     double move_probability)
    : MonteCarloMover(m, "RigidBodyTunneler%1%"), pis_(pis), ref_(ref), k_(k),
      move_probability_(move_probability) {
  for (unsigned i = 0; i < pis.size(); i++) {
    IMP_USAGE_CHECK(RigidBody(m, pis[i]).get_coordinates_are_optimized(),
                    "Rigid body passed to RigidBodyTunneler"
                    << " must be set to be optimized. particle: "
                    << m->get_particle_name(pis[i]));
  }
  IMP_USAGE_CHECK(k >= 0, "k must be positive");
  IMP_USAGE_CHECK(move_probability_ >= 0 && move_probability <= 1,
                  "Probability must be 0<= p <= 1");
}

void RigidBodyTunneler::add_entry_point(Floats fl) {
  IMP_USAGE_CHECK(fl.size() == pis_.size() * 7,
                  "Expected entry point size to be " << (pis_.size() * 7)
                                                     << ", got " << fl.size());
  Coord x;
  unsigned nrbs = pis_.size();
  for (unsigned i = 0; i < nrbs; i++) {
    IMP_Eigen::Vector3d com;
    com << fl[3 * i], fl[3 * i + 1], fl[3 * i + 2];
    x.coms.push_back(com);
    IMP_Eigen::Quaterniond quat(fl[3 * nrbs + 4 * i],
            fl[3 * nrbs + 4 * i + 1],
            fl[3 * nrbs + 4 * i + 2],
            fl[3 * nrbs + 4 * i + 3]);
    quat.normalize();
    x.quats.push_back(quat);
  }
}

double RigidBodyTunneler::get_squared_distance(const Coord& x,
                                               const Coord& y) const {
  double dcom(0), dq(0);
  for (unsigned i = 0; i < x.coms.size(); i++) {
    dcom += x.coms[i].squaredNorm();
    dq += x.quats[i].squaredNorm();
  }
  return dcom + k_ * dq;
}

unsigned RigidBodyTunneler::get_closest_entry_point(
        const RigidBodyTunneler::Coord& x) const {
  double dmin;
  unsigned closest=0;
  for (unsigned i = 0; i < entries_.size(); i++) {
    double dist = get_squared_distance(x, entries_[i]);
    if (i == 0 || dist < dmin) {
      dmin = dist;
      closest = i;
    }
  }
  return closest;
}

RigidBodyTunneler::Coord RigidBodyTunneler::move_point(
        const RigidBodyTunneler::Coord& x,
        const RigidBodyTunneler::Coord& entry,
        const RigidBodyTunneler::Coord& exit) const {
  Coord y;
  for (unsigned i = 0; i < x.coms.size(); i++) {
    y.coms.push_back(x.coms[i] + exit.coms[i] - entry.coms[i]);
    y.quats.push_back(x.quats[i] * exit.quats[i] * entry.quats[i].conjugate());
  }
  return y;
}

RigidBodyTunneler::Coord RigidBodyTunneler::get_coordinates_from_rbs() const {
  // get current reference frame of rbs
  Referential ref(get_model(), ref_);
  //get x
  Coord x;
  for (unsigned i = 0; i < pis_.size(); i++) {
    Referential target(get_model(), pis_[i]);
    x.coms.push_back(ref.get_local_coords(target.get_centroid()));
    x.quats.push_back(ref.get_local_rotation(target.get_rotation()));
  }
  return x;
}

MonteCarloMoverResult RigidBodyTunneler::do_propose() {
  IMP_OBJECT_LOG;
  // check whether mover is usable
  if (entries_.size() < 2)
    IMP_THROW("You must add at least two entry points for this "
              << "mover to be usable.",
              ModelException);
  // see if we are to do the move
  ::boost::uniform_01<double> rand01;
  if (rand01(base::random_number_generator) <= move_probability_) {
    Coord x(get_coordinates_from_rbs());
    // compute the closest entry point
    unsigned closest = get_closest_entry_point(x);
    // get list of other entry points
    std::vector<unsigned> entry_nums;
    for (unsigned i = 0; i < entries_.size(); i++)
      if (i != closest) entry_nums.push_back(i);
    // loop over these entry points
    Coord y;
    unsigned distant;
    do {
      // pick another entry point at random
      ::boost::uniform_int<unsigned> randint(0, entry_nums.size() - 1);
      unsigned dnum = randint(base::random_number_generator);
      distant = entry_nums[dnum];
      // propose move to it
      y = Coord(move_point(x, entries_[closest], entries_[distant]));
      entry_nums.erase(entry_nums.begin()+dnum);
    } while ((!entry_nums.empty()) && (get_closest_entry_point(y) != distant));
    // make move if acceptable
    if (get_closest_entry_point(y) == distant) {
      last_transformations_.clear();
      Referential referential(get_model(), ref_);
      for (unsigned i = 0; i < pis_.size(); i++)
        last_transformations_.push_back(Transformer(
            get_model(), referential, pis_[i], y.coms[i], y.quats[i]));
      IMP_LOG_VERBOSE("proposed move from entry point "
                      << closest << " to " << distant << std::endl);
      std::cout << "proposed move from entry point " << closest << " to "
                << distant << std::endl;
    }
  }
  return MonteCarloMoverResult(pis_, 1.0);
}

void RigidBodyTunneler::do_reject() {
  for (unsigned i = 0; i < last_transformations_.size(); i++)
    last_transformations_[i].undo_transform();
}

Floats RigidBodyTunneler::get_reduced_coordinates(kernel::Model* m,
                                                  kernel::ParticleIndex t,
                                                  kernel::ParticleIndex r) {
  Referential target(m, t), referential(m, r);
  IMP_Eigen::Vector3d com(referential.get_local_coords(target.get_centroid()));
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

Floats RigidBodyTunneler::get_reduced_coordinates(kernel::Model* m,
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
  kernel::ModelObjectsTemp retval;
  for (unsigned i=0; i<pis_.size(); i++)
      retval.push_back(get_model()->get_particle(pis_[i]));
  return retval;
}

IMPCORE_END_NAMESPACE
