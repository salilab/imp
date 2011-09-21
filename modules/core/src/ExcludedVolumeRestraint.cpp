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
#include <IMP/core/TableRefiner.h>
#include <IMP/core/ClosePairsPairScore.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <IMP/core/internal/grid_close_pairs_impl.h>
#include <IMP/core/internal/CoreCloseBipartitePairContainer.h>
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

ExcludedVolumeRestraint::ExcludedVolumeRestraint(SingletonContainer *sc,
                                                 SoftSpherePairScore *ssps,
                                                 ObjectKey ok,
                                                 double s):
  Restraint("ExcludedVolumeRestraint %1%"), sc_(sc),
  initialized_(false),
  ssps_(ssps)
{
  slack_=s;
  key_=ok;
}

void ExcludedVolumeRestraint::
initialize() const {
  IMP_OBJECT_LOG;
  IMP_LOG(TERSE, "Initializing ExcludedVolumeRestraint with "
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
                         (get_model(), cur_list_,
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
                         <SoftSpherePairScore>(get_model(), cur_list_,
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
                         <SoftSpherePairScore>(get_model(), cur_list_,
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

void ExcludedVolumeRestraint::
fill_list() const {
  IMP_OBJECT_LOG;
  IMP_INTERNAL_CHECK(slack_>=0, "Slack must not be negative");
  IMP_LOG(VERBOSE, "filling particle list with slack " << slack_
          << " on " << sc_->get_name());
    xyzrs_backup_.clear();
  rbs_backup_.clear();
  cur_list_.clear();
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
    ret+=ssps_->evaluate_index(get_model(), ParticleIndexPair(cur_list_[i][0],
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
Restraints ExcludedVolumeRestraint::do_create_decomposition() const {
  if (!initialized_) initialize();
  Restraints ret;
  for (unsigned int i=0; i< xyzrs_.size(); ++i) {
    for (unsigned int j=0; j< i; ++j) {
      ret.push_back(create_restraint(ssps_.get(),
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
  for (IMP::compatibility::map<ParticleIndex,
                               ParticleIndexes>::const_iterator it
         = constituents_.begin();
       it != constituents_.end(); ++it) {
    tr->add_particle( IMP::internal::get_particle(get_model(), it->first),
                      IMP::internal::get_particle(get_model(), it->second));
  }
  IMP_NEW(ClosePairsPairScore, cpps, (ssps_, tr, 0));
  for (unsigned int i=0; i< xyzrs_.size(); ++i) {
    for (unsigned int j=0; j< rbs_.size(); ++j) {
      ret.push_back(create_restraint(cpps,
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
      ret.push_back(create_restraint(cpps,
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
    ret.push_back(create_restraint(ssps_.get(),
                                   IMP::internal::get_particle(get_model(),
                                                               cur_list_[i])));
    ret.back()->set_maximum_score(get_maximum_score());
    std::ostringstream oss;
    oss << get_name() << " " << i;
    ret.back()->set_name(oss.str());
  }
  return ret;
}

Restraints
ExcludedVolumeRestraint
::do_create_incremental_decomposition(unsigned int n) const {
  IMP_OBJECT_LOG;
  if (!initialized_) initialize();
  Restraints ret;
  unsigned int chunk= std::max<unsigned int>(1, std::sqrt(sc_->get_number())
                                             /10);
  //std::cout << "Chunks of size " << chunk << std::endl;
  // change chunk here
  IMP::compatibility::checked_vector<ParticleIndexes>
    bins(1, ParticleIndexes());
  for (unsigned int i=0; i< xyzrs_.size(); ++i) {
    bins.back().push_back(xyzrs_[i]);
    if (bins.back().size() >= chunk) {
      bins.push_back(ParticleIndexes());
    }
  }
  for (unsigned int i=0; i< rbs_.size(); ++i) {
    bins.back().insert(bins.back().end(),
                       constituents_[rbs_[i]].begin(),
                       constituents_[rbs_[i]].end());
    if (bins.back().size() >= chunk) {
      bins.push_back(ParticleIndexes());
    }
  }
  if (bins.back().empty()) bins.pop_back();
  internal::CoreListSingletonContainers bincs;
  for (unsigned int i=0; i< bins.size(); ++i) {
    std::ostringstream oss;
    oss << "C"<< i;
    bincs.push_back(new internal::CoreListSingletonContainer(get_model(),
                                                             oss.str()));
    bincs.back()->set_particles(bins[i]);
  }
  IMP_NEW(RigidClosePairsFinder, rcpf, ());
  PairFiltersTemp pfs(pair_filters_begin(),
                      pair_filters_end());
  internal::MovedSingletonContainers mscs(bins.size());
  for (unsigned int i=0; i< bins.size(); ++i) {
    for (unsigned int j=0; j< i; ++j) {
      std::ostringstream oss;
      oss << i << " and " << j;
      IMP_NEW(internal::CoreCloseBipartitePairContainer, ccbpc, (bincs[i].get(),
                                                                 bincs[j].get(),
                                                                 mscs[i],
                                                                 mscs[j],
                                                                 0.0,
                                                                 rcpf.get(),
                                                                 slack_));
      mscs[i]= ccbpc->get_moved_singleton_container(0);
      mscs[j]= ccbpc->get_moved_singleton_container(1);
      IMP_NEW(internal::CorePairsRestraint, ev, (ssps_, ccbpc));
      ccbpc->set_pair_filters(pfs);
      ev->set_name(std::string("R")+oss.str());
      ret.push_back(ev);
    }
    std::ostringstream oss;
    oss << i;
    IMP_NEW(ExcludedVolumeRestraint, evr, (bincs[i], ssps_, key_, slack_));
    evr->set_name(std::string("R")+oss.str());
    evr->set_pair_filters(pfs);
    ret.push_back(evr);
  }
  //std::cout << "Created " << ret.size() << " restraints" << std::endl;
  return ret;
}


IMP_LIST_IMPL(ExcludedVolumeRestraint, PairFilter,
              pair_filter,
              PairFilter*, PairFilters);


IMPCORE_END_NAMESPACE
