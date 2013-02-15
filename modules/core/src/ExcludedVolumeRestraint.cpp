/**
 *  \file ExcludedVolumeRestraint.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/ExcludedVolumeRestraint.h"
#include <IMP/PairContainer.h>
#include <IMP/core/XYZR.h>
#include <IMP/core/FixedRefiner.h>
#include <IMP/internal/InternalPairsRestraint.h>
#include <IMP/core/CoverRefined.h>
#include <IMP/core/HarmonicLowerBound.h>
#include <IMP/core/RigidClosePairsFinder.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/core/PairRestraint.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/core/TableRefiner.h>
#include <IMP/core/ClosePairsPairScore.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <IMP/core/internal/grid_close_pairs_impl.h>
#include <IMP/core/internal/CoreCloseBipartitePairContainer.h>
#include <IMP/core/internal/close_pairs_helpers.h>
#include <IMP/algebra/eigen_analysis.h>
#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

ExcludedVolumeRestraint::ExcludedVolumeRestraint(SingletonContainerAdaptor sc,
                                                 double k, double s,
                                                 std::string name):
  Restraint(sc->get_model(), name), sc_(sc),
  initialized_(false),
  ssps_(new SoftSpherePairScore(k))
{
  slack_=s;
  std::ostringstream oss;
  oss << "ExcludedVolume " << get_name() << " hierarchy " << this;
  key_=ObjectKey(oss.str());
}

ExcludedVolumeRestraint::ExcludedVolumeRestraint(SingletonContainerAdaptor sc,
                                                 SoftSpherePairScore *ssps,
                                                 ObjectKey ok,
                                                 double s):
    Restraint(sc->get_model(), "ExcludedVolumeRestraint %1%"), sc_(sc),
  initialized_(false),
  ssps_(ssps)
{
  slack_=s;
  key_=ok;
}

void ExcludedVolumeRestraint::clear_caches() {
  was_bad_=true;
}

void ExcludedVolumeRestraint::
initialize() const {
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE( "Initializing ExcludedVolumeRestraint with "
          << sc_->get_name()  << std::endl);
  internal::initialize_particles(sc_, key_,
                                 xyzrs_,
                                 rbs_, constituents_,
                                 rbs_backup_, xyzrs_backup_);
  was_bad_=true;
  initialized_=true;
}


double ExcludedVolumeRestraint::
fill_list_if_good(double max) const {
  xyzrs_backup_.clear();
  rbs_backup_.clear();
  was_bad_=true;
  cur_list_.clear();
  double score=0;
  double myslack=0;
  internal::ParticleIndexHelper
      ::fill_close_pairs(internal::ParticleIndexHelper
                         ::get_particle_set(xyzrs_.begin(),
                                            xyzrs_.end(),0),
                         internal::ParticleIndexTraits(get_model(), myslack),
                 internal::ParticleIndexPairSinkWithMax<SoftSpherePairScore>
                         (get_model(), access_pair_filters(),
                          cur_list_,
                          ssps_.get(),
                          nullptr,
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
                         <SoftSpherePairScore>(get_model(),
                                               access_pair_filters(),
                                               cur_list_,
                                               ssps_.get(),
                                               nullptr, score, max,
                                               key_, myslack, constituents_));
  }
  if (score< max) {
    internal::ParticleIndexHelper
      ::fill_close_pairs(internal::ParticleIndexHelper
                         ::get_particle_set(rbs_.begin(),
                                            rbs_.end(),0),
                         internal::ParticleIndexTraits(get_model(), myslack),
                     internal::RigidBodyRigidBodyParticleIndexPairSinkWithMax
                         <SoftSpherePairScore>(get_model(),
                                               access_pair_filters(),
                                               cur_list_,
                                               ssps_.get(),
                                               nullptr, score, max,
                                               key_, myslack, constituents_));
    /* Assume incoherent motion
       if (score < max) {
      was_bad_=false;
      reset_moved();
      }*/
  }
  return score;
}

void ExcludedVolumeRestraint::fill_list() const {
  IMP_OBJECT_LOG;
  internal::fill_list(get_model(), access_pair_filters(), key_,
                      slack_, xyzrs_, rbs_, constituents_,
                      cur_list_);
  was_bad_=false;
}

void ExcludedVolumeRestraint::
reset_moved() const {
  internal::reset_moved(get_model(),
                        xyzrs_, rbs_, constituents_,
                        rbs_backup_, xyzrs_backup_);
}


int ExcludedVolumeRestraint::
get_if_moved() const {
  return internal::get_if_moved(get_model(),
                                slack_,
                                xyzrs_, rbs_, constituents_,
                                rbs_backup_, xyzrs_backup_);
}

double ExcludedVolumeRestraint::
unprotected_evaluate(DerivativeAccumulator *da) const {
  IMP_OBJECT_LOG;
  if (!initialized_) {
    initialize();
  } else {
    IMP_IF_CHECK(base::USAGE) {
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
    reset_moved();
    recomputed=true;
  }
  IMP_CHECK_VARIABLE(recomputed);
  double ret=0;
  for (unsigned int i=0; i< cur_list_.size(); ++i) {
    ret+=ssps_->evaluate_index(get_model(), ParticleIndexPair(cur_list_[i][0],
                                                        cur_list_[i][1]), da);
  }
#if IMP_HAS_CHECKS >= IMP_INTERNAL
    ParticlesTemp all= IMP::get_particles(get_model(), sc_->get_indexes());
    if (all.size() < 3000) {
      double check=0;
      int found=0;
      for (unsigned int i=0; i< all.size(); ++i) {
        for (unsigned int j=0; j< i; ++j) {
          if (!RigidMember::particle_is_instance(all[i])
              || !RigidMember::particle_is_instance(all[j])
              || RigidMember(all[i]).get_rigid_body()
              != RigidMember(all[j]).get_rigid_body()) {
            if( internal::get_filters_contains(get_model(),
                                      PairPredicates(pair_filters_begin(),
                                                  pair_filters_end()),
                                      ParticleIndexPair(all[i]->get_index(),
                                                        all[j]->get_index())))
                continue;
            double cur= ssps_->evaluate(ParticlePair(all[i], all[j]), nullptr);
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
              IMP_CHECK_VARIABLE(cur_found);
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
#endif
  return ret;
}

double ExcludedVolumeRestraint::
unprotected_evaluate_if_good(DerivativeAccumulator *da, double max) const {
  IMP_OBJECT_LOG;
  if (!initialized_) initialize();
  IMP_USAGE_CHECK(!da, "Can't do derivatives");
  IMP_CHECK_CODE(double check=0);
  IMP_CHECK_CODE(ParticlesTemp all= IMP::get_particles(get_model(),
                                                       sc_->get_indexes()));
  IMP_CHECK_CODE(IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
    if (all.size() < 3000) {
      for (unsigned int i=0; i< all.size(); ++i) {
        for (unsigned int j=0; j< i; ++j) {
          if (!RigidMember::particle_is_instance(all[i])
              || !RigidMember::particle_is_instance(all[j])
              || RigidMember(all[i]).get_rigid_body()
              != RigidMember(all[j]).get_rigid_body()) {
            check+= ssps_->evaluate(ParticlePair(all[i], all[j]), nullptr);
          }
        }
      }
    }
    });
  double cur=0;
  for (unsigned int i=0; i< cur_list_.size(); ++i) {
    double c=ssps_->evaluate_index(get_model(),
                                   ParticleIndexPair(cur_list_[i][0],
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
    IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
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

ModelObjectsTemp ExcludedVolumeRestraint
::do_get_inputs() const {
  return internal::get_inputs(get_model(),
                                       sc_, access_pair_filters());
}

Restraints ExcludedVolumeRestraint::do_create_decomposition() const {
  if (!initialized_) initialize();
  Restraints ret;
  for (unsigned int i=0; i< xyzrs_.size(); ++i) {
    for (unsigned int j=0; j< i; ++j) {
      ret.push_back(IMP::create_restraint(ssps_.get(),
                    ParticlePair(IMP::internal::get_particle(get_model(),
                                                             xyzrs_[i]),
                                 IMP::internal::get_particle(get_model(),
                                                             xyzrs_[j]))));
      ret.back()->set_maximum_score(get_maximum_score());
      std::ostringstream oss;
      oss << get_name() << " " << i << " " << j;
      ret.back()->set_name(oss.str());
    }
  }
  IMP_NEW(TableRefiner, tr, ());
  for (IMP::base::map<ParticleIndex,
                               ParticleIndexes>::const_iterator it
         = constituents_.begin();
       it != constituents_.end(); ++it) {
    tr->add_particle( IMP::internal::get_particle(get_model(), it->first),
                      IMP::internal::get_particle(get_model(), it->second));
  }
  IMP_NEW(ClosePairsPairScore, cpps, (ssps_, tr, 0));
  for (unsigned int i=0; i< xyzrs_.size(); ++i) {
    for (unsigned int j=0; j< rbs_.size(); ++j) {
      ret.push_back(IMP::create_restraint(cpps.get(),
                      ParticlePair(IMP::internal::get_particle(get_model(),
                                                               xyzrs_[i]),
                                   IMP::internal::get_particle(get_model(),
                                                               rbs_[j]))));
      ret.back()->set_maximum_score(get_maximum_score());
      std::ostringstream oss;
      oss << get_name() << " " << i << " " << j;
      ret.back()->set_name(oss.str());
    }
  }
  for (unsigned int i=0; i< rbs_.size(); ++i) {
    for (unsigned int j=0; j< i; ++j) {
      ret.push_back(IMP::create_restraint(cpps.get(),
                   ParticlePair(IMP::internal::get_particle(get_model(),
                                                            rbs_[i]),
                       IMP::internal::get_particle(get_model(),rbs_[j]))));
      ret.back()->set_maximum_score(get_maximum_score());
      std::ostringstream oss;
      oss << get_name() << " " << i << " " << j;
      ret.back()->set_name(oss.str());
    }
  }
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]->set_model(get_model());
  }
  return ret;
}

Restraints ExcludedVolumeRestraint::do_create_current_decomposition() const {
  Restraints ret;
  for (unsigned int i=0; i< cur_list_.size(); ++i) {
    Pointer<Restraint> rc=IMP::create_restraint(ssps_.get(),
                                   IMP::internal::get_particle(get_model(),
                                                               cur_list_[i]));
    rc->set_was_used(true);
    double score= rc->unprotected_evaluate(nullptr);
    if (score != 0) {
      rc->set_last_score(score);
      ret.push_back(rc);
      ret.back()->set_maximum_score(get_maximum_score());
      std::ostringstream oss;
      oss << get_name() << " " << i;
      ret.back()->set_name(oss.str());
    }
  }
  return ret;
}


IMP_LIST_IMPL(ExcludedVolumeRestraint, PairFilter,
              pair_filter,
              PairPredicate*, PairPredicates);


IMPCORE_END_NAMESPACE
