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
  oss << "ExcludedVolume " << get_name() << " hierarchy " << this;
  key_=ObjectKey(oss.str());
}

void ExcludedVolumeRestraint::
initialize() const {
  IMP_OBJECT_LOG;
  IMP_LOG(TERSE, "Initializing ExcludedVolumeRestraint with "
          << sc_->get_number_of_particles() << " particles"
          << std::endl);
  IMP_FOREACH_SINGLETON(sc_, {
      if (RigidMember::particle_is_instance(_1)) {
        RigidBody rb=RigidMember(_1).get_rigid_body();
        ParticleIndex pi= rb.get_particle_index();
        rbs_.push_back(rb.get_particle_index());
        if (constituents_.find(pi) == constituents_.end()) {
          constituents_.insert(std::make_pair(pi, ParticleIndexes(1,
                                                 _1->get_index())));
        } else {
          constituents_[pi].push_back(_1->get_index());
        }
      } else {
        xyzrs_.push_back(_1->get_index());
      }
    });
  std::sort(rbs_.begin(), rbs_.end());
  rbs_.erase(std::unique(rbs_.begin(), rbs_.end()), rbs_.end());
  for (unsigned int i=0; i < rbs_.size(); ++i) {
    internal::get_rigid_body_hierarchy(RigidBody(get_model(), rbs_[i]),
                                       constituents_[rbs_[i]],
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
  double myslack=0;
  internal::ParticleIndexHelper
      ::fill_close_pairs(internal::ParticleIndexHelper
                         ::get_particle_set(xyzrs_.begin(),
                                            xyzrs_.end(),0),
                         internal::ParticleIndexTraits(get_model(), myslack),
                 internal::ParticleIndexPairSinkWithMax<SoftSpherePairScore>
                         (get_model(), cur_list_,
                          ssps_.get(),
                          NULL,
                          score,
                          max));
  if (score < max) {
    internal::ParticleIndexHelper
      ::fill_close_pairs(internal::ParticleIndexHelper
                         ::get_particle_set(xyzrs_.begin(),
                                            xyzrs_.end(),0),
                         internal::ParticleIndexHelper
                         ::get_particle_set(rbs_.begin(),
                                            rbs_.end(),1),
                         internal::ParticleIndexTraits(get_model(), myslack),
                         internal::RigidBodyParticleParticleIndexPairSinkWithMax
                         <SoftSpherePairScore>(get_model(), cur_list_,
                                               ssps_.get(),
                                               NULL, score, max,
                                               key_, myslack, constituents_));
  }
  if (score< max) {
    internal::ParticleIndexHelper
      ::fill_close_pairs(internal::ParticleIndexHelper
                         ::get_particle_set(rbs_.begin(),
                                            rbs_.end(),0),
                         internal::ParticleIndexTraits(get_model(), myslack),
                     internal::RigidBodyRigidBodyParticleIndexPairSinkWithMax
                         <SoftSpherePairScore>(get_model(), cur_list_,
                                               ssps_.get(),
                                               NULL, score, max,
                                               key_, myslack, constituents_));
    /* Assume incoherent motion
       if (score < max) {
      was_bad_=false;
      reset_moved();
      }*/
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
  internal::ParticleIndexHelper
      ::fill_close_pairs(internal::ParticleIndexHelper
                         ::get_particle_set(xyzrs_.begin(),
                                            xyzrs_.end(),0),
                         internal::ParticleIndexTraits(get_model(), slack_),
                   internal::ParticleIndexPairSink(get_model(), cur_list_));
  internal::ParticleIndexHelper
    ::fill_close_pairs(internal::ParticleIndexHelper
                       ::get_particle_set(rbs_.begin(),
                                          rbs_.end(),0),
                       internal::ParticleIndexHelper
                       ::get_particle_set(xyzrs_.begin(),
                                          xyzrs_.end(),1),
                       internal::ParticleIndexTraits(get_model(), slack_),
               internal::RigidBodyParticleParticleIndexPairSink(get_model(),
                                                                   cur_list_,
                                                                   key_,
                                                                   slack_,
                                                            constituents_));
  internal::ParticleIndexHelper
    ::fill_close_pairs(internal::ParticleIndexHelper
                       ::get_particle_set(rbs_.begin(),
                                          rbs_.end(),0),
                       internal::ParticleIndexTraits(get_model(), slack_),
               internal::RigidBodyRigidBodyParticleIndexPairSink(get_model(),
                                                                    cur_list_,
                                                                    key_,
                                                                    slack_,
                                                             constituents_));
  IMP_LOG(VERBOSE, "found " << cur_list_.size() << std::endl);
  reset_moved();
  was_bad_=false;
}

void ExcludedVolumeRestraint::
reset_moved() const {
  xyzrs_backup_.resize(xyzrs_.size());
  for (unsigned int i=0; i< xyzrs_.size(); ++i) {
    xyzrs_backup_[i]= get_model()->get_sphere(xyzrs_[i]).get_center();
  }
  rbs_backup_.resize(rbs_.size());
  for (unsigned int i=0; i< rbs_.size(); ++i) {
    rbs_backup_[i]= RigidBody(get_model(), rbs_[i]).get_reference_frame()
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
      double diffc2= square(get_model()->get_sphere(xyzrs_[i]).get_center()[j]
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
      double diffc2= square(get_model()->get_sphere(rbs_[i]).get_center()[j]
                            - rbs_backup_[i].get_translation()[j]);
      diff2+=diffc2;
      if (diff2> s22) {
        return true;
      }
    }
    algebra::Rotation3D nrot=RigidBody(get_model(),
                                       rbs_[i]).get_reference_frame()
      .get_transformation_to().get_rotation();
    algebra::Rotation3D diffrot
      = rbs_backup_[i].get_rotation().get_inverse()*nrot;
    double angle= algebra::get_axis_and_angle(diffrot).second;
    double drot= std::abs(angle*get_model()->get_sphere(rbs_[i]).get_radius());
    if (s22 < square(drot)+drot*std::sqrt(diff2)+ diff2) {
      return true;
    }
  }
  return false;
}

double ExcludedVolumeRestraint::
unprotected_evaluate(DerivativeAccumulator *da) const {
  IMP_OBJECT_LOG;
  if (!initialized_) {
    initialize();
  } else {
    IMP_IF_CHECK(USAGE) {
      IMP_FOREACH_SINGLETON(sc_, {
          if (RigidMember::particle_is_instance(_1)) {
            RigidBody rb= RigidMember(_1).get_rigid_body();
            using IMP::operator<<;
            IMP_USAGE_CHECK(std::find(rbs_.begin(), rbs_.end(),
                                      rb.get_particle()->get_index())
                            != rbs_.end(),
                    "You cannot change the contents of the singleton container "
                    << "passed to ExcludedVolume after the first evaluate."
                    << " Found unexpected rigid body " << rb->get_name()
                    << " not in " << rbs_);
      } else {
            IMP_USAGE_CHECK(std::find(xyzrs_.begin(), xyzrs_.end(),
                                      _1->get_index())
                            != xyzrs_.end(),
               "You cannot change the contents of the singleton container "
               << "passed to ExcludedVolume after the first evaluate."
               << " Found unexpected particle " << _1->get_name());
          }
        });
    }
  }
  bool recomputed=false;
  if (was_bad_ || get_if_moved()>0) {
    cur_list_.clear();
    fill_list();
    recomputed=true;
  }
  double ret=0;
  for (unsigned int i=0; i< cur_list_.size(); ++i) {
    ret+=ssps_->evaluate(get_model(), ParticleIndexPair(cur_list_[i][0],
                                                        cur_list_[i][1]), da);
  }
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
                                       ParticleIndexPair(all[i]->get_index(),
                                                         all[j]->get_index()))
                != cur_list_.end()
                || std::find(cur_list_.begin(), cur_list_.end(),
                             ParticleIndexPair(all[j]->get_index(),
                                               all[i]->get_index()))
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
  double cur=0;
  for (unsigned int i=0; i< cur_list_.size(); ++i) {
    double c=ssps_->evaluate(get_model(), ParticleIndexPair(cur_list_[i][0],
                                                       cur_list_[i][1]), da);
    cur+=c;
    max-=c;
    if (max<0) {
      IMP_INTERNAL_CHECK(all.size() >=3000 || check > max,
                         "I think it is bad, but it isn't: "
                         << cur << " vs " << check);
      return cur;
    };
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
  for (unsigned int i=0; i< rbs_.size(); ++i) {
    ret.push_back(get_model()->get_particle(rbs_[i]));
  }
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
    ret[i]= new PairRestraint(ssps_,
               ParticlePair(get_model()->get_particle(cur_list_[i][0]),
                          get_model()->get_particle(cur_list_[i][1])));
  }
  return ret;
}

IMP_LIST_PLURAL_IMPL(ExcludedVolumeRestraint, PairFilter,
                     PairFilters, pair_filter,
                     pair_filters,
                     PairFilter*, PairFilters, ,,);


IMPCORE_END_NAMESPACE
