/**
 *  \file RigidClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/RigidClosePairsFinder.h"
#include "IMP/core/GridClosePairsFinder.h"
#include "IMP/core/internal/MovedSingletonContainer.h"
#include "IMP/core/rigid_bodies.h"
#include "IMP/core/internal/rigid_body_tree.h"
#include "IMP/core/XYZ.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/SingletonContainer.h>
#include <IMP/macros.h>
#include <boost/unordered_map.hpp>
#include <IMP/algebra/eigen_analysis.h>
#include <boost/unordered_set.hpp>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

RigidClosePairsFinder::RigidClosePairsFinder(ClosePairsFinder *cpf)
    : ClosePairsFinder("RigidCPF") {
  std::ostringstream oss;
  oss << "RigidClosePairsFinderHiearchy " << this;
  k_ = ObjectKey(oss.str());
  if (cpf) {
    cpf_ = cpf;
  } else {
    cpf_ = Pointer<ClosePairsFinder>(new GridClosePairsFinder());
  }
}

namespace {
ParticlesTemp get_rigid_bodies(Model *m,
                               const ParticleIndexes &pis) {
  boost::unordered_set<Particle *> rets;
  for (unsigned int i = 0; i < pis.size(); ++i) {
    if (RigidMember::get_is_setup(m, pis[i])) {
      rets.insert(RigidMember(m, pis[i]).get_rigid_body());
    }
  };
  return ParticlesTemp(rets.begin(), rets.end());
}

/*ParticlesTemp get_non_rigid(SingletonContainer *sc) {
  ParticlesTemp ret;
  IMP_FOREACH_SINGLETON(sc, {
      if (!RigidMember::get_is_setup(_1)) {
        ret.push_back(_1);
      }
    });
  return ret;
  }*/
typedef boost::unordered_map<ParticleIndex, ParticleIndexes>
    RBM;
void divvy_up_particles(Model *m, const ParticleIndexes &ps,
                        ParticleIndexes &out, RBM &members) {
  IMP_IF_CHECK(USAGE) {
    boost::unordered_set<ParticleIndex> ups(ps.begin(), ps.end());
    IMP_USAGE_CHECK(ups.size() == ps.size(),
                    "Duplicate particles in input: " << ups.size()
                                                     << "!= " << ps.size());
  }
  for (unsigned int i = 0; i < ps.size(); ++i) {
    if (RigidMember::get_is_setup(m, ps[i])) {
      RigidBody rb = RigidMember(m, ps[i]).get_rigid_body();
      if (members.find(rb) == members.end()) {
        out.push_back(rb);
      }
      members[rb].push_back(ps[i]);
    } else {
      out.push_back(ps[i]);
    }
  }
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    ParticleIndexes check_out = out;
    std::sort(check_out.begin(), check_out.end());
    check_out.erase(std::unique(check_out.begin(), check_out.end()),
                    check_out.end());
    IMP_INTERNAL_CHECK(check_out.size() == out.size(),
                       "Values added twice: " << check_out.size() << " vs "
                                              << out.size() << ": " << out
                                              << " vs " << check_out);
  }
  /*std::cout << "Found " << members.size() << " rigid bodies and "
            << insc->get_number_of_particles()
            << " total particles" << std::endl;*/
}

void check_particles(Model *m, const ParticleIndexes &ps) {
  IMP_IF_CHECK(USAGE) {
    for (ParticleIndexes::const_iterator it = ps.begin();
         it != ps.end(); ++it) {
      if (RigidBody::get_is_setup(m, *it) &&
          !m->get_has_attribute(XYZR::get_radius_key(), *it)) {
        IMP_WARN("Particle " << m->get_particle_name(*it) << " is a rigid body "
                             << "but does not have a radius. "
                             << "Collision detection is unlikely to work."
                             << std::endl);
      }
    }
  }
}
}

ParticleIndexPairs RigidClosePairsFinder::get_close_pairs(
    Model *m, const ParticleIndexes &pia,
    const ParticleIndexes &pib) const {
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE("Rigid add_close_pairs called with "
                << pia.size() << " and " << pib.size() << " and distance "
                << get_distance() << std::endl);
  check_particles(m, pia);
  check_particles(m, pib);
  RBM ma, mb;
  ParticleIndexes fa, fb;
  divvy_up_particles(m, pia, fa, ma);
  divvy_up_particles(m, pib, fb, mb);
  ParticleIndexPairs ppt = cpf_->get_close_pairs(m, fa, fb);
  ParticleIndexPairs ret;
  for (ParticleIndexPairs::const_iterator it = ppt.begin();
       it != ppt.end(); ++it) {
    // skip within one rigid body
    if (it->get(0) == it->get(1)) continue;
    ParticleIndexes ps0, ps1;
    if (ma.find(it->get(0)) != ma.end()) {
      ps0 = ma.find(it->get(0))->second;
    }
    if (mb.find(it->get(1)) != mb.end()) {
      ps1 = mb.find(it->get(1))->second;
    }
    if (ps0.empty() && ps1.empty()) {
      ret.push_back(*it);
    } else {
      ret += get_close_pairs(m, it->get(0), it->get(1), ps0, ps1);
    }
  }
  return ret;
}

ParticleIndexPairs RigidClosePairsFinder::get_close_pairs(
    Model *m, const ParticleIndexes &pia) const {
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE("Adding close pairs from " << pia.size() << " particles."
                                           << std::endl);
  check_particles(m, pia);
  RBM map;
  ParticleIndexes fa;
  divvy_up_particles(m, pia, fa, map);
  ParticleIndexPairs ppt = cpf_->get_close_pairs(m, fa);
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    for (unsigned int i = 0; i < ppt.size(); ++i) {
      IMP_INTERNAL_CHECK(ppt[i][0] != ppt[i][1],
                         "Pair of one returned: " << ppt[i]);
    }
  }
  ParticleIndexPairs ret;
  for (ParticleIndexPairs::const_iterator it = ppt.begin();
       it != ppt.end(); ++it) {
    ParticleIndexes ps0, ps1;
    IMP_LOG_VERBOSE("Processing close pair " << *it << std::endl);
    if (map.find(it->get(0)) != map.end()) {
      ps0 = map.find(it->get(0))->second;
    }
    if (map.find(it->get(1)) != map.end()) {
      ps1 = map.find(it->get(1))->second;
    }
    if (ps0.empty() && ps1.empty()) {
      ret.push_back(*it);
    } else {
      ret += get_close_pairs(m, it->get(0), it->get(1), ps0, ps1);
    }
  }
  return ret;
}

ParticleIndexPairs RigidClosePairsFinder::get_close_pairs(
    Model *m, ParticleIndex a, ParticleIndex b,
    const ParticleIndexes &ma,
    const ParticleIndexes &mb) const {
  IMP_INTERNAL_CHECK(a != b, "Can't pass equal particles");
  internal::RigidBodyHierarchy *da = nullptr, *db = nullptr;
  ParticleIndexPairs out;
  if (ma.size() > 0) {
    da = internal::get_rigid_body_hierarchy(RigidBody(m, a), ma, k_);
    IMP_INTERNAL_CHECK(da, "No hierarchy gotten");
  }
  if (mb.size() > 0) {
    db = internal::get_rigid_body_hierarchy(RigidBody(m, b), mb, k_);
    IMP_INTERNAL_CHECK(db, "No hierarchy gotten");
  }
  /*IMP_INTERNAL_CHECK(RigidBody::get_is_setup(a)==(da!=nullptr),
                     "Rigid body does not imply hierarchy");
  IMP_INTERNAL_CHECK(RigidBody::get_is_setup(b)==(db!=nullptr),
  "Rigid body does not imply hierarchy");*/
  if (da && db) {
    out = IMP::get_indexes(internal::close_pairs(m, da, db, get_distance()));
  } else if (da) {
    ParticlesTemp pt =
        internal::close_particles(m, da, XYZR(m, b), get_distance());
    out.resize(pt.size());
    for (unsigned int i = 0; i < pt.size(); ++i) {
      out[i] = ParticleIndexPair(pt[i]->get_index(), b);
    }
  } else if (db) {
    ParticlesTemp pt =
        internal::close_particles(m, db, XYZR(m, a), get_distance());
    out.resize(pt.size());
    for (unsigned int i = 0; i < pt.size(); ++i) {
      out[i] = ParticleIndexPair(a, pt[i]->get_index());
    }
  }
  return out;
}

IntPairs RigidClosePairsFinder::get_close_pairs(
    const algebra::BoundingBox3Ds &bas,
    const algebra::BoundingBox3Ds &bbs) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  cpf_->set_distance(get_distance());
  return cpf_->get_close_pairs(bas, bbs);
}

IntPairs RigidClosePairsFinder::get_close_pairs(
    const algebra::BoundingBox3Ds &bas) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  cpf_->set_distance(get_distance());
  return cpf_->get_close_pairs(bas);
}

/*
std::pair<algebra::Sphere3D, algebra::Sphere3D>
RigidClosePairsFinder::get_close_sphere_pair(Particle *a,
Particle *b) const {

}*/

ModelObjectsTemp RigidClosePairsFinder::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret = IMP::get_particles(m, pis);
  ParticlesTemp rbs = get_rigid_bodies(m, pis);
  ParticleIndexes all_pis = pis;
  all_pis += get_indexes(rbs);
  ret += rbs;
  for (unsigned int i = 0; i < rbs.size(); ++i) {
    RigidMembers rms = RigidBody(rbs[i]).get_rigid_members();
    for (unsigned i = 0; i < rms.size(); ++i) {
      all_pis.push_back(rms[i].get_particle_index());
    }
    ret += rms;
  }
  if (get_number_of_pair_filters() > 0) {
    for (PairFilterConstIterator it = pair_filters_begin();
         it != pair_filters_end(); ++it) {
      ret += (*it)->get_inputs(m, all_pis);
    }
  }
  return ret;
}

internal::MovedSingletonContainer *
RigidClosePairsFinder::get_moved_singleton_container(SingletonContainer *in,
                                                     double threshold) const {
  return new internal::RigidMovedSingletonContainer(in, threshold);
}

IMPCORE_END_NAMESPACE
