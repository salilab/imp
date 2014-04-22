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

RigidBodyTunneler::RigidBodyTunneler(kernel::Model *m,
                                     kernel::ParticleIndexes pis,
                                     kernel::ParticleIndex ref, double k,
                                     double move_probability)
    : MonteCarloMover(m, "RigidBodyTunneler%1%"), pis_(pis),
      ref_(ref), k_(k), move_probability_(move_probability) {
  for (unsigned i = 0; i < pis.size(); i++) {
    reset_stats();
    IMP_USAGE_CHECK(RigidBody(m, pis[i]
                             ).get_coordinates_are_optimized(),
                    "Rigid body passed to RigidBodyTunneler"
                    << " must be set to be optimized. particle: "
                    << m->get_particle_name(pis[i]));
  }
  IMP_USAGE_CHECK(k >= 0, "k must be positive");
  IMP_USAGE_CHECK(move_probability_ >= 0 && move_probability <= 1,
                  "Probability must be 0<= p <= 1");
}

void RigidBodyTunneler::reset_stats() {
  num_calls_ = 0;
  num_proposed_ = 0;
  num_rejected_ = 0;
  num_impossible_ = 0;
}

void RigidBodyTunneler::add_entry_point(Floats fl) {
  IMP_USAGE_CHECK(fl.size() == pis_.size() * 7,
                  "Expected entry point size to be " << (pis_.size() * 7)
                                                     << ", got " << fl.size());
  internal::Coord x(fl);
  entries_.push_back(x);
}

unsigned RigidBodyTunneler::get_closest_entry_point(const internal::Coord
                                                    & x) const {
  double dmin;
  unsigned closest = 0;
  for (unsigned i = 0; i < entries_.size(); i++) {
    double dist = internal::get_squared_distance(x, entries_[i], k_);
    if (i == 0 || dist < dmin) {
      dmin = dist;
      closest = i;
    }
  }
  return closest;
}

MonteCarloMoverResult RigidBodyTunneler::do_propose() {
  IMP_OBJECT_LOG;
  // check whether mover is usable
  if (entries_.size() < 2)
    IMP_THROW("You must add at least two entry points for this "
              << "mover to be usable.",
              ModelException);
  num_calls_++;
  // see if we are to do the move
  ::boost::uniform_01<double> rand01;
  if (rand01(base::random_number_generator) <= move_probability_) {
    IMP_LOG_TERSE("will try to move" << std::endl);
    internal::Coord x(
        internal::get_coordinates_from_rbs(get_model(), pis_, ref_));
    IMP_LOG_VERBOSE("x is at " << x << std::endl);
    // compute the closest entry point
    unsigned closest = get_closest_entry_point(x);
    IMP_LOG_TERSE("closest entry point is number " << closest << std::endl);
    IMP_LOG_VERBOSE("with coordinates " << entries_[closest] << std::endl);
    // get list of other entry points
    std::vector<unsigned> entry_nums;
    for (unsigned i = 0; i < entries_.size(); i++)
      if (i != closest) entry_nums.push_back(i);
    // loop over these entry points
    internal::Coord y, transform_coord;
    unsigned distant;
    do {
      // pick another entry point at random
      IMP_LOG_TERSE("New iteration, entry_nums.size() == " << entry_nums.size()
                                                           << std::endl);
      ::boost::uniform_int<unsigned> randint(0, entry_nums.size() - 1);
      unsigned dnum = randint(base::random_number_generator);
      distant = entry_nums[dnum];
      IMP_LOG_TERSE("distant entry point is number " << distant << std::endl);
      IMP_LOG_VERBOSE("with coordinates " << entries_[distant] << std::endl);
      // propose move to it
      for (unsigned i = 0; i < x.coms.size(); i++) {
        transform_coord.coms.push_back(entries_[distant].coms[i]
                                       - entries_[closest].coms[i]);
        y.coms.push_back(transform_coord.coms.back() + x.coms[i]);
        transform_coord.quats.push_back(
            internal::pick_positive(entries_[closest].quats[i].conjugate()
                                    * entries_[distant].quats[i]));
        y.quats.push_back(
            internal::pick_positive(transform_coord.quats.back() * x.quats[i]));
      }
      IMP_LOG_VERBOSE("transform is " << transform_coord << std::endl);
      IMP_LOG_VERBOSE("y is at " << y << std::endl);
      IMP_LOG_TERSE("y is closest to " << get_closest_entry_point(y)
                                       << std::endl);
      // remove that element from list
      entry_nums.erase(entry_nums.begin() + dnum);
    } while ((!entry_nums.empty()) && (get_closest_entry_point(y) != distant));
    // make move if acceptable
    if (get_closest_entry_point(y) == distant) {
      last_transformations_.clear();
      internal::Referential referential(get_model(), ref_);
      for (unsigned i = 0; i < pis_.size(); i++)
        last_transformations_.push_back(new internal::Transformer(
            get_model(), referential, pis_[i], transform_coord.coms[i],
            transform_coord.quats[i]));
      IMP_LOG_TERSE("proposed move from entry point " << closest << " to "
                                                      << distant << std::endl);
      IMP_USAGE_CHECK(internal::get_squared_distance(
                          y, internal::get_coordinates_from_rbs(
                                 get_model(), pis_, ref_), k_) < 1e-5,
                      "Weird things happened here!");
      num_proposed_++;
    } else {
      IMP_LOG_TERSE("no move was possible" << std::endl);
      num_impossible_++;
    }
  }
  return MonteCarloMoverResult(pis_, 1.0);
}

void RigidBodyTunneler::do_reject() {
  bool was_reset = false;
  for (unsigned i = 0; i < last_transformations_.size(); i++) {
    bool reset = last_transformations_[i].undo_transform();
    was_reset = was_reset || reset;
  }
  if (was_reset) {
    IMP_LOG_TERSE("move rejected" << std::endl);
    num_rejected_++;
  }
}

Floats RigidBodyTunneler::get_reduced_coordinates(kernel::Model* m,
                                                  kernel::ParticleIndex t,
                                                  kernel::ParticleIndex r) {
  internal::Referential target(m, t), referential(m, r);
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
  internal::Referential target(m, pi);
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
  for (unsigned i = 0; i < pis_.size(); i++)
    retval.push_back(get_model()->get_particle(pis_[i]));
  return retval;
}

IMPCORE_END_NAMESPACE
