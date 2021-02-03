/**
 *  \file CloseBipartitePairContainer.cpp
 *  \brief internal implementation of close pair container
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/internal/CloseBipartitePairContainer.h"
#include <IMP/core/BoxSweepClosePairsFinder.h>
#include <IMP/core/GridClosePairsFinder.h>
#include <IMP/container/ListPairContainer.h>
#include <IMP/core/internal/close_pairs_helpers.h>
#include <IMP/warning_macros.h>
#include <IMP/PairModifier.h>
#include <algorithm>

IMPCONTAINER_BEGIN_INTERNAL_NAMESPACE

IMP_LIST_IMPL(CloseBipartitePairContainer, PairFilter, pair_filter,
              PairFilter *, PairFilters);

CloseBipartitePairContainer::CloseBipartitePairContainer(
    SingletonContainer *a, SingletonContainer *b, double distance, double slack,
    std::string name)
    : P(a->get_model(), name) {
  std::ostringstream oss;
  oss << "BCPC " << get_name() << " hierarchy " << this;
  ObjectKey key = ObjectKey(oss.str());
  initialize(a, b, IMP::get_invalid_index<ParticleIndexTag>(),
             IMP::get_invalid_index<ParticleIndexTag>(), distance,
             slack, key);
  // initialize can get called more than once
  score_state_ = new SS(this);
}

CloseBipartitePairContainer::CloseBipartitePairContainer(
    SingletonContainer *a, SingletonContainer *b,
    ParticleIndex cover_a, ParticleIndex cover_b,
    ObjectKey key, double distance, double slack, std::string name)
    : P(a->get_model(), name) {
  initialize(a, b, cover_a, cover_b, distance, slack, key);

  score_state_ = new SS(this);
}

void CloseBipartitePairContainer::initialize(SingletonContainer *a,
                                                 SingletonContainer *b,
                                                 ParticleIndex cover_a,
                                                 ParticleIndex cover_b,
                                                 double distance, double slack,
                                                 ObjectKey key) {
  IMP_OBJECT_LOG;
  slack_ = slack;
  distance_ = distance;
  key_ = key;
  sc_[0] = a;
  sc_[1] = b;
  were_close_ = false;
  reset_ = false;
  covers_[0] = cover_a;
  covers_[1] = cover_b;
  for (unsigned int i = 0; i < 2; ++i) {
    core::internal::initialize_particles(sc_[i], key_, xyzrs_[i], rbs_[i],
                                         constituents_, rbs_backup_sphere_[i],
                                         rbs_backup_rot_[i], xyzrs_backup_[i]);
  }
}

ModelObjectsTemp CloseBipartitePairContainer::do_get_inputs() const {
  ModelObjectsTemp ret;
  ret.push_back(sc_[0]);
  ret.push_back(sc_[1]);
  ret.push_back(score_state_);
  return ret;
}

ModelObjectsTemp CloseBipartitePairContainer::get_score_state_inputs()
    const {
  ModelObjectsTemp ret;
  ret += core::internal::get_inputs(get_model(), sc_[0], access_pair_filters());
  ret += core::internal::get_inputs(get_model(), sc_[1], access_pair_filters());
  if (covers_[0] != IMP::get_invalid_index<ParticleIndexTag>()) {
    ret.push_back(get_model()->get_particle(covers_[0]));
    ret.push_back(get_model()->get_particle(covers_[1]));
  }
  ret.push_back(sc_[0]);
  ret.push_back(sc_[1]);
  return ret;
}

void CloseBipartitePairContainer::do_score_state_before_evaluate() {
  IMP_OBJECT_LOG;
  IMP_IF_LOG(VERBOSE) {
    algebra::Sphere3Ds coords[2];
    for (unsigned int i = 0; i < 2; ++i) {
      for (unsigned int j = 0; j < xyzrs_[i].size(); ++j) {
        coords[i].push_back(get_model()->get_sphere(xyzrs_[i][j]));
      }
    }
    Floats distances;
    for (unsigned int i = 0; i < coords[0].size(); ++i) {
      for (unsigned int j = 0; j < coords[1].size(); ++j) {
        distances.push_back(algebra::get_distance(coords[0][i], coords[1][j]));
      }
    }
    IMP_LOG_VERBOSE(xyzrs_[0] << " " << coords[0] << " " << xyzrs_backup_[0]
                              << std::endl);
    IMP_LOG_VERBOSE(xyzrs_[1] << " " << coords[1] << " " << xyzrs_backup_[1]
                              << std::endl);
    IMP_LOG_VERBOSE(distances << std::endl);
  }
  if (covers_[0] == IMP::get_invalid_index<ParticleIndexTag>() ||
      algebra::get_distance(get_model()->get_sphere(covers_[0]),
                            get_model()->get_sphere(covers_[1])) < distance_ ||
      reset_) {
    if (!reset_ && were_close_ &&
        !core::internal::get_if_moved(get_model(), slack_, xyzrs_[0], rbs_[0],
                                constituents_, rbs_backup_sphere_[0],
                                rbs_backup_rot_[0], xyzrs_backup_[0]) &&
        !core::internal::get_if_moved(get_model(), slack_, xyzrs_[1], rbs_[1],
                                constituents_, rbs_backup_sphere_[1],
                                rbs_backup_rot_[1], xyzrs_backup_[1])) {
      IMP_LOG_TERSE("Nothing to update" << std::endl);
      // all ok
    } else {
      // rebuild
      IMP_LOG_TERSE("Recomputing bipartite close pairs list." << std::endl);
      core::internal::reset_moved(get_model(), xyzrs_[0], rbs_[0], constituents_,
                            rbs_backup_sphere_[0], rbs_backup_rot_[0],
                            xyzrs_backup_[0]);
      core::internal::reset_moved(get_model(), xyzrs_[1], rbs_[1], constituents_,
                            rbs_backup_sphere_[1], rbs_backup_rot_[1],
                            xyzrs_backup_[1]);
      ParticleIndexPairs pips;
      core::internal::fill_list(get_model(), access_pair_filters(), key_,
                          2 * slack_ + distance_, xyzrs_, rbs_, constituents_,
                          pips);
      reset_ = false;

      swap(pips);

      IMP_LOG_VERBOSE("List is " << get_access() << std::endl);
      IMP_IF_CHECK(USAGE_AND_INTERNAL) {
        ParticleIndexes sc0p = sc_[0]->get_indexes();
        ParticleIndexes sc1p = sc_[1]->get_indexes();
        ParticleIndexPairs unfound;
        for (unsigned int i = 0; i < sc0p.size(); ++i) {
          core::XYZR d0(get_model(), sc0p[i]);
          for (unsigned int j = 0; j < sc1p.size(); ++j) {
            core::XYZR d1(get_model(), sc1p[j]);
            double dist = get_distance(d0, d1);
            if (dist < .9 * (distance_ + 2 * slack_)) {
              ParticleIndexPair pip(sc0p[i], sc1p[j]);
              bool filtered = false;
              IMP_CHECK_VARIABLE(filtered);
              for (unsigned int i = 0; i < get_number_of_pair_filters(); ++i) {
                if (get_pair_filter(i)->get_value_index(get_model(), pip)) {
                  filtered = true;
                  break;
                }
              }
              if (!filtered &&
                  std::find(get_access().begin(), get_access().end(), pip) ==
                      get_access().end()) {
                unfound.push_back(pip);
              }
            }
          }
        }
        IMP_INTERNAL_CHECK(unfound.empty(),
                           "Missing particle pairs: " << unfound);
      }
    }
    were_close_ = true;
  } else {
    IMP_LOG_TERSE("Covers are well separated." << std::endl);
    ParticleIndexPairs none;
    swap(none);
  }
  IMP_LOG_VERBOSE("List is " << get_access() << std::endl);
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    ParticleIndexes sc0p = sc_[0]->get_indexes();
    ParticleIndexes sc1p = sc_[1]->get_indexes();
    ParticleIndexPairs unfound;
    for (unsigned int i = 0; i < sc0p.size(); ++i) {
      core::XYZR d0(get_model(), sc0p[i]);
      for (unsigned int j = 0; j < sc1p.size(); ++j) {
        core::XYZR d1(get_model(), sc1p[j]);
        double dist = get_distance(d0, d1);
        if (dist < .9 * distance_) {
          ParticleIndexPair pip(sc0p[i], sc1p[j]);
          bool filtered = false;
          IMP_CHECK_VARIABLE(filtered);
          for (unsigned int i = 0; i < get_number_of_pair_filters(); ++i) {
            if (get_pair_filter(i)->get_value_index(get_model(), pip)) {
              filtered = true;
              break;
            }
          }
          if (!filtered && std::find(get_access().begin(), get_access().end(),
                                     pip) == get_access().end()) {
            unfound.push_back(pip);
          }
        }
      }
    }
    IMP_INTERNAL_CHECK(unfound.empty(), "Missing particle pairs: " << unfound);
  }
}

ParticleIndexes CloseBipartitePairContainer::get_all_possible_indexes()
    const {
  ParticleIndexes ret = sc_[0]->get_indexes();
  ret += sc_[1]->get_indexes();
  return ret;
}

ParticleIndexPairs CloseBipartitePairContainer::get_range_indexes() const {
  ParticleIndexes pis = sc_[0]->get_range_indexes();
  ParticleIndexes pjs = sc_[1]->get_range_indexes();
  ParticleIndexPairs ret;
  ret.reserve(pis.size() * pjs.size());
  for (unsigned int i = 0; i < pis.size(); ++i) {
    for (unsigned int j = 0; j < pjs.size(); ++j) {
      ret.push_back(ParticleIndexPair(pis[i], pjs[j]));
    }
  }
  return ret;
}

IMPCONTAINER_END_INTERNAL_NAMESPACE
