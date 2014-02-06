/**
 *  \file RigidClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
    cpf_ = base::Pointer<ClosePairsFinder>(new GridClosePairsFinder());
  }
}

namespace {
ParticlesTemp get_rigid_bodies(kernel::Model *m,
                               const kernel::ParticleIndexes &pis) {
  IMP::base::set<kernel::Particle *> rets;
  for (unsigned int i = 0; i < pis.size(); ++i) {
    if (RigidMember::get_is_setup(m, pis[i])) {
      rets.insert(RigidMember(m, pis[i]).get_rigid_body());
    }
  };
  return kernel::ParticlesTemp(rets.begin(), rets.end());
}

/*ParticlesTemp get_non_rigid(SingletonContainer *sc) {
  kernel::ParticlesTemp ret;
  IMP_FOREACH_SINGLETON(sc, {
      if (!RigidMember::get_is_setup(_1)) {
        ret.push_back(_1);
      }
    });
  return ret;
  }*/
typedef boost::unordered_map<kernel::ParticleIndex, kernel::ParticleIndexes>
    RBM;
void divvy_up_particles(kernel::Model *m, const kernel::ParticleIndexes &ps,
                        kernel::ParticleIndexes &out, RBM &members) {
  IMP_IF_CHECK(base::USAGE) {
    base::set<kernel::ParticleIndex> ups(ps.begin(), ps.end());
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
  IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
    kernel::ParticleIndexes check_out = out;
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

void check_particles(kernel::Model *m, const kernel::ParticleIndexes &ps) {
  IMP_IF_CHECK(base::USAGE) {
    for (kernel::ParticleIndexes::const_iterator it = ps.begin();
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
    kernel::Model *m, const kernel::ParticleIndexes &pia,
    const kernel::ParticleIndexes &pib) const {
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE("Rigid add_close_pairs called with "
                << pia.size() << " and " << pib.size() << " and distance "
                << get_distance() << std::endl);
  check_particles(m, pia);
  check_particles(m, pib);
  RBM ma, mb;
  kernel::ParticleIndexes fa, fb;
  divvy_up_particles(m, pia, fa, ma);
  divvy_up_particles(m, pib, fb, mb);
  kernel::ParticleIndexPairs ppt = cpf_->get_close_pairs(m, fa, fb);
  kernel::ParticleIndexPairs ret;
  for (kernel::ParticleIndexPairs::const_iterator it = ppt.begin();
       it != ppt.end(); ++it) {
    // skip within one rigid body
    if (it->get(0) == it->get(1)) continue;
    kernel::ParticleIndexes ps0, ps1;
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
    kernel::Model *m, const kernel::ParticleIndexes &pia) const {
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE("Adding close pairs from " << pia.size() << " particles."
                                           << std::endl);
  check_particles(m, pia);
  RBM map;
  kernel::ParticleIndexes fa;
  divvy_up_particles(m, pia, fa, map);
  kernel::ParticleIndexPairs ppt = cpf_->get_close_pairs(m, fa);
  IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
    for (unsigned int i = 0; i < ppt.size(); ++i) {
      IMP_INTERNAL_CHECK(ppt[i][0] != ppt[i][1],
                         "Pair of one returned: " << ppt[i]);
    }
  }
  kernel::ParticleIndexPairs ret;
  for (kernel::ParticleIndexPairs::const_iterator it = ppt.begin();
       it != ppt.end(); ++it) {
    kernel::ParticleIndexes ps0, ps1;
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
    kernel::Model *m, kernel::ParticleIndex a, kernel::ParticleIndex b,
    const kernel::ParticleIndexes &ma,
    const kernel::ParticleIndexes &mb) const {
  IMP_INTERNAL_CHECK(a != b, "Can't pass equal particles");
  internal::RigidBodyHierarchy *da = nullptr, *db = nullptr;
  kernel::ParticleIndexPairs out;
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
    kernel::ParticlesTemp pt =
        internal::close_particles(m, da, XYZR(m, b), get_distance());
    out.resize(pt.size());
    for (unsigned int i = 0; i < pt.size(); ++i) {
      out[i] = kernel::ParticleIndexPair(pt[i]->get_index(), b);
    }
  } else if (db) {
    kernel::ParticlesTemp pt =
        internal::close_particles(m, db, XYZR(m, a), get_distance());
    out.resize(pt.size());
    for (unsigned int i = 0; i < pt.size(); ++i) {
      out[i] = kernel::ParticleIndexPair(a, pt[i]->get_index());
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
RigidClosePairsFinder::get_close_sphere_pair(kernel::Particle *a,
kernel::Particle *b) const {

}*/

ModelObjectsTemp RigidClosePairsFinder::do_get_inputs(
    kernel::Model *m, const kernel::ParticleIndexes &pis) const {
  kernel::ModelObjectsTemp ret = IMP::get_particles(m, pis);
  kernel::ParticlesTemp rbs = get_rigid_bodies(m, pis);
  ret += rbs;
  for (unsigned int i = 0; i < rbs.size(); ++i) {
    ret += RigidBody(rbs[i]).get_members();
  }
  if (get_number_of_pair_filters() > 0) {
    for (PairFilterConstIterator it = pair_filters_begin();
         it != pair_filters_end(); ++it) {
      for (unsigned int i = 0; i < ret.size(); ++i) {
        ret += (*it)->get_inputs(m, pis);
      }
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
