/**
 *  \file internal/close_pairs_helpers.h
 *  \brief utilities for rigid bodies.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_CLOSE_PAIRS_HELPERS_H
#define IMPCORE_INTERNAL_CLOSE_PAIRS_HELPERS_H

#include <IMP/core/core_config.h>
#include "../BoxSweepClosePairsFinder.h"
#include "../GridClosePairsFinder.h"
#include "grid_close_pairs_impl.h"
#include "rigid_body_tree.h"
#include <IMP/container_macros.h>
#include "../XYZR.h"
#include <IMP/base/warning_macros.h>
#include <IMP/SingletonContainer.h>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <algorithm>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

inline ClosePairsFinder *default_cpf(unsigned int) {
  return new GridClosePairsFinder();
}

/*inline IMP::internal::InternalListPairContainer *get_list(PairContainer *pc) {
  return dynamic_cast<IMP::internal::InternalListPairContainer *>(pc);
  }*/

template <class C>
inline void filter_close_pairs(C *c, kernel::ParticleIndexPairs &ps) {
  for (typename C::PairFilterConstIterator it = c->pair_filters_begin();
       it != c->pair_filters_end(); ++it) {
    (*it)->remove_if_not_equal(c->get_model(), ps, 0);
  }
}

inline void fix_order(kernel::ParticleIndexPairs &pips) {
  for (unsigned int i = 0; i < pips.size(); ++i) {
    if (pips[i][0] > pips[i][1]) {
      pips[i] = kernel::ParticleIndexPair(pips[i][1], pips[i][0]);
    }
  }
}
/*
// Check that they are unique by checking order
struct SameParticle {
  kernel::ParticleIndexes ppt_;
  SameParticle(const kernel::ParticleIndexes &ppt): ppt_(ppt){
    std::sort(ppt_.begin(), ppt_.end());
  }
  SameParticle(){}
  bool operator()(const kernel::ParticleIndexPair& pp) {
    if (ppt_.empty()) {
      return pp[0] == pp[1];
    } else {
      if (binary_search(ppt_.begin(), ppt_.end(), pp[0])
          && binary_search(ppt_.begin(), ppt_.end(), pp[1])) {
        return !(pp[0] < pp[1]);
      } else {
        return false;
      }
    }
  }
  };*/

inline bool get_are_close(kernel::Model *m, const PairPredicates &filters,
                          kernel::ParticleIndex a, kernel::ParticleIndex b,
                          double distance) {
  XYZ da(m, a);
  XYZ db(m, b);
  Float ra = XYZR(m, a).get_radius();
  Float rb = XYZR(m, b).get_radius();
  Float sr = ra + rb + distance;
  for (unsigned int i = 0; i < 3; ++i) {
    double delta = std::abs(da.get_coordinate(i) - db.get_coordinate(i));
    if (delta >= sr) {
      return false;
    }
  }
  return get_interiors_intersect(
             algebra::Sphere3D(da.get_coordinates(), ra + distance),
             algebra::Sphere3D(db.get_coordinates(), rb)) &&
         !get_filters_contains(m, filters, kernel::ParticleIndexPair(a, b));
}

struct FarParticle {
  double d_;
  kernel::Model *m_;
  FarParticle(kernel::Model *m, double d) : d_(d), m_(m) {}
  bool operator()(const kernel::ParticleIndexPair &pp) const {
    return !get_are_close(m_, PairPredicates(), pp[0], pp[1], d_);
  }
};

inline void filter_far(kernel::Model *m, kernel::ParticleIndexPairs &c,
                       double d) {
  c.erase(std::remove_if(c.begin(), c.end(), FarParticle(m, d)), c.end());
}

struct InList {
  static IntKey key_;
  const kernel::ParticlesTemp &ps_;
  InList(const kernel::ParticlesTemp &ps) : ps_(ps) {}
  static InList create(kernel::ParticlesTemp &ps) {
    /*for (unsigned int i=0; i< ps.size(); ++i) {
      ps[i]->add_attribute(key_, 1);
      }*/
    std::sort(ps.begin(), ps.end());
    return InList(ps);
  }
  static void destroy(InList &) {
    /*for (unsigned int i=0; i< il.ps_.size(); ++i) {
      il.ps_[i]->remove_attribute(key_);
      }*/
  }
  bool operator()(const kernel::ParticlePair &pp) const {
    if (std::binary_search(ps_.begin(), ps_.end(), pp[0]))
      return true;
    else if (std::binary_search(ps_.begin(), ps_.end(), pp[1]))
      return true;
    return false;
    // return pp[0]->has_attribute(key_) || pp[1]->has_attribute(key_);
  }
};

inline void reset_moved(
    kernel::Model *m, kernel::ParticleIndexes &xyzrs_,
    kernel::ParticleIndexes &rbs_,
    boost::unordered_map<kernel::ParticleIndex,
                         kernel::ParticleIndexes> & /*constituents_*/,
    algebra::Sphere3Ds &rbs_backup_sphere_,
    algebra::Rotation3Ds &rbs_backup_rot_, algebra::Sphere3Ds &xyzrs_backup_) {
  xyzrs_backup_.resize(xyzrs_.size());
  for (unsigned int i = 0; i < xyzrs_.size(); ++i) {
    xyzrs_backup_[i] = m->get_sphere(xyzrs_[i]);
  }
  rbs_backup_sphere_.resize(rbs_.size());
  rbs_backup_rot_.resize(rbs_.size());
  for (unsigned int i = 0; i < rbs_.size(); ++i) {
    rbs_backup_sphere_[i] = m->get_sphere(rbs_[i]);
    rbs_backup_rot_[i] = RigidBody(m, rbs_[i])
                             .get_reference_frame()
                             .get_transformation_to()
                             .get_rotation();
  }
}

inline void initialize_particles(
    SingletonContainer *sc, ObjectKey key, kernel::ParticleIndexes &xyzrs_,
    kernel::ParticleIndexes &rbs_,
    boost::unordered_map<kernel::ParticleIndex, kernel::ParticleIndexes> &
        constituents_,
    algebra::Sphere3Ds &rbs_backup_sphere_,
    algebra::Rotation3Ds &rbs_backup_rot_, algebra::Sphere3Ds &xyzrs_backup_,
    bool use_rigid_bodies = true) {
  IMP_IF_CHECK(base::USAGE) {
    kernel::ParticleIndexes pis = sc->get_indexes();
    boost::unordered_set<kernel::ParticleIndex> spis(pis.begin(), pis.end());
    IMP_USAGE_CHECK(pis.size() == spis.size(),
                    "Duplicate particle indexes in input");
  }
  IMP_IF_CHECK(base::USAGE) {
    kernel::ParticlesTemp pis = sc->get();
    boost::unordered_set<kernel::Particle *> spis(pis.begin(), pis.end());
    IMP_USAGE_CHECK(pis.size() == spis.size(), "Duplicate particles in input");
  }
  // constituents_.clear();
  xyzrs_.clear();
  rbs_.clear();
  using IMP::operator<<;
  IMP_CONTAINER_FOREACH(SingletonContainer, sc, {
    kernel::Model *m = sc->get_model();
    IMP_LOG_VERBOSE("Processing " << m->get_particle_name(_1) << " (" << _1
                                  << ")" << std::endl);
    if (use_rigid_bodies && RigidMember::get_is_setup(m, _1)) {
      RigidBody rb = RigidMember(m, _1).get_rigid_body();
      kernel::ParticleIndex pi = rb.get_particle_index();
      rbs_.push_back(rb.get_particle_index());
      if (constituents_.find(pi) == constituents_.end()) {
        constituents_.insert(
            std::make_pair(pi, kernel::ParticleIndexes(1, _1)));
      } else {
        constituents_[pi].push_back(_1);
      }
      IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
        kernel::ParticleIndexes cur = constituents_[pi];
        IMP_USAGE_CHECK(std::find(cur.begin(), cur.end(), pi) == cur.end(),
                        "A rigid body can't be its own constituent.");
        boost::unordered_set<kernel::ParticleIndex> scur(cur.begin(),
                                                         cur.end());
        IMP_USAGE_CHECK(cur.size() == scur.size(),
                        "Duplicate constituents for "
                            << sc->get_model()->get_particle(pi)->get_name()
                            << ": " << cur);
      }
    } else {
      xyzrs_.push_back(_1);
    }
  });
  std::sort(rbs_.begin(), rbs_.end());
  rbs_.erase(std::unique(rbs_.begin(), rbs_.end()), rbs_.end());
  for (unsigned int i = 0; i < rbs_.size(); ++i) {
    internal::get_rigid_body_hierarchy(RigidBody(sc->get_model(), rbs_[i]),
                                       constituents_[rbs_[i]], key);
  }
  reset_moved(sc->get_model(), xyzrs_, rbs_, constituents_, rbs_backup_sphere_,
              rbs_backup_rot_, xyzrs_backup_);
  xyzrs_backup_.clear();
  rbs_backup_sphere_.clear();
  rbs_backup_rot_.clear();
  IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
    for (boost::unordered_map<kernel::ParticleIndex,
                              kernel::ParticleIndexes>::const_iterator it =
             constituents_.begin();
         it != constituents_.end(); ++it) {
      kernel::ParticleIndexes cur = it->second;
      IMP_USAGE_CHECK(std::find(cur.begin(), cur.end(), it->first) == cur.end(),
                      "A rigid body cann't be its own constituent.");
      boost::unordered_set<kernel::ParticleIndex> scur(cur.begin(), cur.end());
      IMP_USAGE_CHECK(
          cur.size() == scur.size(),
          "Duplicate constituents for "
              << sc->get_model()->get_particle(it->first)->get_name() << ": "
              << Showable(cur));
    }
  }
}

inline bool get_if_moved(
    kernel::Model *m, double slack_, const kernel::ParticleIndexes &xyzrs_,
    const kernel::ParticleIndexes &rbs_,
    const boost::unordered_map<kernel::ParticleIndex,
                               kernel::ParticleIndexes> & /*constituents_*/,
    const algebra::Sphere3Ds &rbs_backup_sphere_,
    const algebra::Rotation3Ds &rbs_backup_rot_,
    const algebra::Sphere3Ds &xyzrs_backup_) {
  IMP_INTERNAL_CHECK(xyzrs_.size() == xyzrs_backup_.size(),
                     "Backup is not a backup");
  const double s22 = algebra::get_squared(slack_);
  for (unsigned int i = 0; i < xyzrs_.size(); ++i) {
    double diff2 = algebra::get_squared_distance(
        m->get_sphere(xyzrs_[i]).get_center(), xyzrs_backup_[i].get_center());
    if (diff2 >= s22) return true;
    double rdiff = std::abs(m->get_sphere(xyzrs_[i]).get_radius() -
                            xyzrs_backup_[i].get_radius());
    if (rdiff == 0) continue;
    double diff = std::sqrt(diff2);
    if (algebra::get_squared(rdiff + diff) >= s22) return true;
  }
  for (unsigned int i = 0; i < rbs_.size(); ++i) {
    double diff2 =
        algebra::get_squared_distance(m->get_sphere(rbs_[i]).get_center(),
                                      rbs_backup_sphere_[i].get_center());
    if (diff2 >= s22) {
      return true;
    }
    double rdiff = std::abs(m->get_sphere(rbs_[i]).get_radius() -
                            rbs_backup_sphere_[i].get_radius());
    if (rdiff != 0) {
      double diff = std::sqrt(diff2);
      if (algebra::get_squared(rdiff + diff) >= s22) return true;
    }
    algebra::Rotation3D nrot = RigidBody(m, rbs_[i])
                                   .get_reference_frame()
                                   .get_transformation_to()
                                   .get_rotation();
    algebra::Rotation3D diffrot = rbs_backup_rot_[i].get_inverse() * nrot;
    double angle = algebra::get_axis_and_angle(diffrot).second;
    double drot = std::abs(angle * m->get_sphere(rbs_[i]).get_radius());
    if (s22 < algebra::get_squared(drot) + drot * std::sqrt(diff2) + diff2) {
      return true;
    }

// This check touches particles it shouldn't when the members are not
// all being used for collision detection.
#if 0
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      core::RigidBody rbs(m, rbs_[i]);
      core::RigidMembers rms = rbs.get_members();
      algebra::Transformation3D tr(rbs_backup_[i]);
      algebra::ReferenceFrame3D old(tr);
      algebra::ReferenceFrame3D cur = rbs.get_reference_frame();
      for (unsigned int i = 0; i < rms.size(); ++i) {
        algebra::Vector3D local = rms[i].get_internal_coordinates();
        algebra::Vector3D oldv = old.get_global_coordinates(local);
        algebra::Vector3D newv = cur.get_global_coordinates(local);
        double dist = get_distance(oldv, newv);
        IMP_CHECK_VARIABLE(dist);
        IMP_INTERNAL_CHECK(dist < slack_ + .1,
                           "Particle moved further than expected "
                               << dist << " > " << slack_ << " for "
                               << Showable(rms[i].get_particle()));
      }
    }
#endif
  }
  return false;
}

inline void fill_list(
    kernel::Model *m, const PairPredicates &filters, ObjectKey key_,
    double slack_, kernel::ParticleIndexes &xyzrs_,
    kernel::ParticleIndexes &rbs_,
    boost::unordered_map<kernel::ParticleIndex, kernel::ParticleIndexes> &
        constituents_,
    kernel::ParticleIndexPairs &cur_list_) {
  IMP_INTERNAL_CHECK(slack_ >= 0, "Slack must not be negative");
  /*IMP_LOG_VERBOSE( "filling particle list with slack " << slack_
    << " on " << sc_->get_name());*/
  cur_list_.clear();
  internal::ParticleIndexHelper::fill_close_pairs(
      internal::ParticleIndexHelper::get_particle_set(xyzrs_.begin(),
                                                      xyzrs_.end(), 0),
      internal::ParticleIndexTraits(m, slack_),
      internal::ParticleIndexPairSink(m, filters, cur_list_));
  internal::ParticleIndexHelper::fill_close_pairs(
      internal::ParticleIndexHelper::get_particle_set(rbs_.begin(), rbs_.end(),
                                                      0),
      internal::ParticleIndexHelper::get_particle_set(xyzrs_.begin(),
                                                      xyzrs_.end(), 1),
      internal::ParticleIndexTraits(m, slack_),
      internal::RigidBodyParticleParticleIndexPairSink(
          m, filters, cur_list_, key_, slack_, constituents_));
  internal::ParticleIndexHelper::fill_close_pairs(
      internal::ParticleIndexHelper::get_particle_set(rbs_.begin(), rbs_.end(),
                                                      0),
      internal::ParticleIndexTraits(m, slack_),
      internal::RigidBodyRigidBodyParticleIndexPairSink(
          m, filters, cur_list_, key_, slack_, constituents_));
  IMP_LOG_VERBOSE("found " << cur_list_.size() << std::endl);
}

inline void fill_list(
    kernel::Model *m, const PairPredicates &filters, ObjectKey key_,
    double slack_, kernel::ParticleIndexes xyzrs_[],
    kernel::ParticleIndexes rbs_[],
    boost::unordered_map<kernel::ParticleIndex, kernel::ParticleIndexes> &
        constituents_,
    kernel::ParticleIndexPairs &cur_list_) {
  IMP_INTERNAL_CHECK(slack_ >= 0, "Slack must not be negative");
  /*IMP_LOG_VERBOSE( "filling particle list with slack " << slack_
    << " on " << sc_->get_name());*/
  cur_list_.clear();
  internal::ParticleIndexHelper::fill_close_pairs(
      internal::ParticleIndexHelper::get_particle_set(xyzrs_[0].begin(),
                                                      xyzrs_[0].end(), 0),
      internal::ParticleIndexHelper::get_particle_set(xyzrs_[1].begin(),
                                                      xyzrs_[1].end(), 1),
      internal::ParticleIndexTraits(m, slack_),
      internal::ParticleIndexPairSink(m, filters, cur_list_));
  internal::ParticleIndexHelper::fill_close_pairs(
      internal::ParticleIndexHelper::get_particle_set(rbs_[0].begin(),
                                                      rbs_[0].end(), 0),
      internal::ParticleIndexHelper::get_particle_set(xyzrs_[1].begin(),
                                                      xyzrs_[1].end(), 1),
      internal::ParticleIndexTraits(m, slack_),
      internal::RigidBodyParticleParticleIndexPairSink(
          m, filters, cur_list_, key_, slack_, constituents_));
  internal::ParticleIndexHelper::fill_close_pairs(
      internal::ParticleIndexHelper::get_particle_set(xyzrs_[0].begin(),
                                                      xyzrs_[0].end(), 0),
      internal::ParticleIndexHelper::get_particle_set(rbs_[1].begin(),
                                                      rbs_[1].end(), 1),
      internal::ParticleIndexTraits(m, slack_),
      internal::ParticleRigidBodyParticleIndexPairSink(
          m, filters, cur_list_, key_, slack_, constituents_));
  internal::ParticleIndexHelper::fill_close_pairs(
      internal::ParticleIndexHelper::get_particle_set(rbs_[0].begin(),
                                                      rbs_[0].end(), 0),
      internal::ParticleIndexHelper::get_particle_set(rbs_[1].begin(),
                                                      rbs_[1].end(), 1),
      internal::ParticleIndexTraits(m, slack_),
      internal::RigidBodyRigidBodyParticleIndexPairSink(
          m, filters, cur_list_, key_, slack_, constituents_));
  IMP_LOG_VERBOSE("found " << cur_list_.size() << std::endl);
}

IMPCOREEXPORT kernel::ParticlesTemp get_input_particles(
    kernel::Model *, kernel::SingletonContainer *sc_,
    const kernel::PairPredicates &filters_);

IMPCOREEXPORT kernel::ModelObjectsTemp get_inputs(
    kernel::Model *, kernel::SingletonContainer *sc_,
    const kernel::PairPredicates &filters_);

IMPCORE_END_INTERNAL_NAMESPACE

#endif /* IMPCORE_INTERNAL_CLOSE_PAIRS_HELPERS_H */
