/**
 *  \file ExcludedVolumeRestraint.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/ExcludedVolumeRestraint.h"
#include <IMP/PairContainer.h>
#include <IMP/core/XYZR.h>
#include <IMP/core/FixedRefiner.h>
#include <IMP/core/internal/CorePairsRestraint.h>
#include <IMP/core/CoverRefined.h>
#include <IMP/core/HarmonicLowerBound.h>
#include <IMP/core/RigidClosePairsFinder.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/PairRestraint.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <IMP/core/internal/grid_close_pairs_impl.h>
#include <IMP/core/internal/close_pairs_helpers.h>
#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

ExcludedVolumeRestraint::ExcludedVolumeRestraint(SingletonContainer *sc,
                                                 double k, double s):
  Restraint("ExcludedVolumeRestraint %1%"), sc_(sc),
  initialized_(false),
  ssps_(new SoftSpherePairScore(k))
{
  slack_=s;
  std::ostringstream oss;
  oss << "hierarchy " << this;
  key_=ObjectKey(oss.str());
}

void ExcludedVolumeRestraint::
initialize() const {
  compatibility::map<RigidBody, ParticlesTemp> constituents;
  IMP_FOREACH_SINGLETON(sc_, {
      if (RigidMember::particle_is_instance(_1)) {
        RigidBody rb=RigidMember(_1).get_rigid_body();
        rbs_.push_back(rb);
        if (constituents.find(rb) == constituents.end()) {
          constituents.insert(std::make_pair(rb, ParticlesTemp(1,_1)));
        } else {
          constituents[rb].push_back(_1);
        }
      } else {
        xyzrs_.push_back(_1);
      }
    });
  std::sort(rbs_.begin(), rbs_.end());
  rbs_.erase(std::unique(rbs_.begin(), rbs_.end()), rbs_.end());
  for (unsigned int i=0; i < rbs_.size(); ++i) {
    internal::get_rigid_body_hierarchy(RigidBody(rbs_[i]),
                                       constituents[RigidBody(rbs_[i])],
                                       key_);
  }
  reset_moved();
  was_bad_=true;
  initialized_=true;
  xyzrs_backup_.clear();
  rbs_backup_.clear();
}


double ExcludedVolumeRestraint::
fill_list_if_good(double max) const {
  xyzrs_backup_.clear();
  rbs_backup_.clear();
  was_bad_=true;
  IMP_INTERNAL_CHECK(cur_list_.empty(), "List not empty");
  double score=0;
  algebra::BoundingBox3D bb;
  for (unsigned int i=0; i< xyzrs_.size(); ++i) {
    bb+= core::XYZ(xyzrs_[i]).get_coordinates();
  }
  internal::ParticleHelper
      ::fill_close_pairs(internal::ParticleHelper
                         ::get_particle_set(xyzrs_.begin(),
                                            xyzrs_.end(),
                                            internal::ParticleID(),
                                            internal::ParticleCenter(),
                                            internal::ParticleRadius()),
                         internal::ParticleClose(0),
                         0, bb, 0,
                         internal::ParticlePairSinkWithMax<SoftSpherePairScore>
                         (cur_list_,
                          ssps_.get(),
                          NULL,
                          score,
                          max));
  if (score < max) {
    for (unsigned int i=0; i< rbs_.size(); ++i) {
      bb+= core::XYZ(rbs_[i]).get_coordinates();
    }
    internal::ParticleHelper
      ::fill_close_pairs(internal::ParticleHelper
                         ::get_particle_set(xyzrs_.begin(),
                                            xyzrs_.end(),
                                            internal::ParticleID(),
                                            internal::ParticleCenter(),
                                            internal::ParticleRadius()),
                         internal::ParticleHelper
                         ::get_particle_set(rbs_.begin(),
                                            rbs_.end(),
                                            internal::ParticleID(),
                                            internal::ParticleCenter(),
                                            internal::ParticleRadius()),
                         internal::ParticleClose(0),
                         0, bb, 0,
                         internal::RigidBodyParticleParticlePairSinkWithMax
                         <SoftSpherePairScore>(cur_list_, ssps_.get(),
                                               NULL, score, max,
                                               key_, slack_));
  }
  if (score< max) {
    internal::ParticleHelper
      ::fill_close_pairs(internal::ParticleHelper
                         ::get_particle_set(rbs_.begin(),
                                            rbs_.end(),
                                          internal::ParticleID(),
                                            internal::ParticleCenter(),
                                            internal::ParticleRadius()),
                         internal::ParticleClose(0),
                         0, bb, 0,
                         internal::RigidBodyRigidBodyParticlePairSinkWithMax
                         <SoftSpherePairScore>(cur_list_, ssps_.get(),
                                               NULL, score, max,
                                               key_, slack_));
    if (score < max) {
      was_bad_=false;
    }
  }
  return score;
}

void ExcludedVolumeRestraint::
fill_list() const {
  IMP_OBJECT_LOG;
  IMP_INTERNAL_CHECK(slack_>=0, "Slack must not be negative");
  IMP_LOG(VERBOSE, "filling particle list with slack " << slack_
          << " on " << sc_->get_number_of_particles() << " partcles");
    xyzrs_backup_.clear();
  rbs_backup_.clear();
  IMP_INTERNAL_CHECK(cur_list_.empty(), "List not empty");
  algebra::BoundingBox3D bb;
  for (unsigned int i=0; i< xyzrs_.size(); ++i) {
    bb+= core::XYZ(xyzrs_[i]).get_coordinates();
  }
  internal::ParticleHelper
      ::fill_close_pairs(internal::ParticleHelper
                         ::get_particle_set(xyzrs_.begin(),
                                            xyzrs_.end(),
                                            internal::ParticleID(),
                                            internal::ParticleCenter(),
                                            internal::ParticleRadius()),
                         internal::ParticleClose(slack_),
                         slack_, bb, 0,
                         internal::ParticlePairSink(cur_list_));
  for (unsigned int i=0; i< rbs_.size(); ++i) {
    bb+= core::XYZ(rbs_[i]).get_coordinates();
  }
  internal::ParticleHelper
    ::fill_close_pairs(internal::ParticleHelper
                       ::get_particle_set(rbs_.begin(),
                                          rbs_.end(),
                                          internal::ParticleID(),
                                          internal::ParticleCenter(),
                                          internal::ParticleRadius()),
                       internal::ParticleHelper
                       ::get_particle_set(xyzrs_.begin(),
                                          xyzrs_.end(),
                                          internal::ParticleID(),
                                          internal::ParticleCenter(),
                                          internal::ParticleRadius()),
                       internal::ParticleClose(slack_),
                       slack_, bb, 0,
                       internal::RigidBodyParticleParticlePairSink(cur_list_,
                                                                   key_,
                                                                   slack_));
  internal::ParticleHelper
    ::fill_close_pairs(internal::ParticleHelper
                       ::get_particle_set(rbs_.begin(),
                                          rbs_.end(),
                                          internal::ParticleID(),
                                          internal::ParticleCenter(),
                                          internal::ParticleRadius()),
                       internal::ParticleClose(slack_),
                       slack_, bb, 0,
                       internal::RigidBodyRigidBodyParticlePairSink(cur_list_,
                                                                    key_,
                                                                    slack_));
  IMP_LOG(VERBOSE, "found " << cur_list_.size() << std::endl);
  reset_moved();
  was_bad_=false;
}

void ExcludedVolumeRestraint::
reset_moved() const {
  xyzrs_backup_.resize(xyzrs_.size());
  for (unsigned int i=0; i< xyzrs_.size(); ++i) {
    xyzrs_backup_[i]= XYZ(xyzrs_[i]).get_coordinates();
  }
  rbs_backup_.resize(rbs_.size());
  for (unsigned int i=0; i< rbs_.size(); ++i) {
    rbs_backup_[i]= RigidBody(rbs_[i]).get_reference_frame()
      .get_transformation_to();
  }
}


int ExcludedVolumeRestraint::
get_if_moved() const {
  IMP_INTERNAL_CHECK(xyzrs_.size()== xyzrs_backup_.size(),
                     "Backup is not a backup");
  const double s22= square(slack_/2);
  for (unsigned int i=0; i< xyzrs_.size(); ++i) {
    double diff2=0;
    for (unsigned int j=0; j< 3; ++j) {
      double diffc2= square(XYZ(xyzrs_[i]).get_coordinate(j)
                            - xyzrs_backup_[i][j]);
      diff2+=diffc2;
      if (diff2> s22) {
        return true;
      }
    }
  }
  for (unsigned int i=0; i< rbs_.size(); ++i) {
    double diff2=0;
    for (unsigned int j=0; j< 3; ++j) {
      double diffc2= square(XYZ(rbs_[i]).get_coordinate(j)
                            - rbs_backup_[i].get_translation()[j]);
      diff2+=diffc2;
      if (diff2> s22) {
        return true;
      }
    }
    algebra::Rotation3D nrot=RigidBody(rbs_[i]).get_reference_frame()
      .get_transformation_to().get_rotation();
    algebra::Rotation3D diffrot
      = rbs_backup_[i].get_rotation().get_inverse()*nrot;
    double angle= algebra::get_axis_and_angle(diffrot).second;
    double drot= std::abs(angle*XYZR(rbs_[i]).get_radius());
    if (s22 < square(drot)+drot*std::sqrt(diff2)+ diff2) {
      return true;
    }
  }
  return false;
}

double ExcludedVolumeRestraint::
unprotected_evaluate(DerivativeAccumulator *da) const {
  IMP_OBJECT_LOG;
  if (!initialized_) initialize();
  bool recomputed=false;
  if (was_bad_ || get_if_moved()>0) {
    fill_list();
    recomputed=true;
  }
  double ret= ssps_->evaluate(cur_list_, da);
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    ParticlesTemp all= sc_->get_particles();
    if (all.size() < 3000) {
      double check=0;
      int found=0;
      for (unsigned int i=0; i< all.size(); ++i) {
        for (unsigned int j=0; j< i; ++j) {
          if (!RigidMember::particle_is_instance(all[i])
              || !RigidMember::particle_is_instance(all[j])
              || RigidMember(all[i]).get_rigid_body()
              != RigidMember(all[j]).get_rigid_body()) {
            double cur= ssps_->evaluate(ParticlePair(all[i], all[j]), NULL);
            check+=cur;
            if (cur > 0) {
              ++found;
              bool cur_found=std::find(cur_list_.begin(), cur_list_.end(),
                                       ParticlePair(all[i], all[j]))
                != cur_list_.end()
                || std::find(cur_list_.begin(), cur_list_.end(),
                                       ParticlePair(all[j], all[i]))
                != cur_list_.end();
              IMP_INTERNAL_CHECK(cur_found, "Pair " << all[i]->get_name()
                                 << " " << all[j]->get_name() << " is close "
                                 << " but not in list: " << recomputed
                                 << " they are " << XYZR(all[i])
                                 << " and " << XYZR(all[j])
                                 << " at distance "
                                 << get_distance(XYZR(all[i]),
                                                 XYZR(all[j])));
            }
          }
        }
      }
      IMP_INTERNAL_CHECK((check-ret)<.1*(check+ret)+.1, "Bad value computed: "
                         << check << " vs " << ret << " on "
                         << cur_list_.size() << " pairs vs "
                         << found << " was " << recomputed << std::endl);
    }
  }
  return ret;
}

double ExcludedVolumeRestraint::
unprotected_evaluate_if_good(DerivativeAccumulator *da, double max) const {
  IMP_OBJECT_LOG;
  if (!initialized_) initialize();
  IMP_USAGE_CHECK(!da, "Can't do derivatives");
  IMP_CHECK_CODE(double check=0);
  IMP_CHECK_CODE(ParticlesTemp all= sc_->get_particles());
  IMP_CHECK_CODE(IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    if (all.size() < 3000) {
      for (unsigned int i=0; i< all.size(); ++i) {
        for (unsigned int j=0; j< i; ++j) {
          if (!RigidMember::particle_is_instance(all[i])
              || !RigidMember::particle_is_instance(all[j])
              || RigidMember(all[i]).get_rigid_body()
              != RigidMember(all[j]).get_rigid_body()) {
            check+= ssps_->evaluate(ParticlePair(all[i], all[j]), NULL);
          }
        }
      }
    }
    });
  double cur= ssps_->evaluate_if_good(cur_list_, da, max);
  if (cur > max) {
    IMP_INTERNAL_CHECK(all.size() >=3000 || check > max,
                       "I think it is bad, but it isn't: "
                       << cur << " vs " << check);
    return cur;
  }
  if (was_bad_ || get_if_moved()>0) {
    double ret= fill_list_if_good(max);
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      if (ret > max) {
        IMP_INTERNAL_CHECK(all.size() >=3000 || check > max,
                       "I think it is bad, but it isn't 2: "
                       << cur << " vs " << check);
      } else {
        IMP_INTERNAL_CHECK((check-ret)<.1*(check+ret)+.1,
                           "Bad value computed 2: "
                         << check << " vs " << ret << std::endl);
      }
    }
    return ret;
  } else {
    IMP_INTERNAL_CHECK((check-cur)<.1*(check+cur), "Bad value computed 3: "
                         << check << " vs " << cur << std::endl);
    return cur;
  }
}


void ExcludedVolumeRestraint::
do_show(std::ostream &) const {
}

ParticlesTemp ExcludedVolumeRestraint
::get_input_particles() const {
  if (!initialized_) initialize();
  ParticlesTemp ret= sc_->get_contained_particles();
  ret.insert(ret.end(), rbs_.begin(), rbs_.end());
  return ret;
}

ContainersTemp ExcludedVolumeRestraint
::get_input_containers() const {
  return ContainersTemp(1, sc_);
}

Restraints ExcludedVolumeRestraint
::get_instantaneous_decomposition() const {
  unprotected_evaluate_if_good(NULL, get_maximum_score());
  Restraints ret(cur_list_.size());
  for (unsigned int i=0; i< cur_list_.size(); ++i) {
    ret[i]= new PairRestraint(ssps_, cur_list_[i]);
  }
  return ret;
}

IMP_LIST_PLURAL_IMPL(ExcludedVolumeRestraint, PairFilter,
                     PairFilters, pair_filter,
                     pair_filters,
                     PairFilter*, PairFilters, ,,);


IMPCORE_END_NAMESPACE
