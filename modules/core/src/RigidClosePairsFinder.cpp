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
#include <IMP/algebra/eigen_analysis.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

RigidClosePairsFinder
::RigidClosePairsFinder(ClosePairsFinder *cpf):
  ClosePairsFinder("RigidCPF") {
  std::ostringstream oss;
  oss << "RigidClosePairsFinderHiearchy " << this;
  k_= ObjectKey(oss.str());
  if (cpf) {
    cpf_=cpf;
  } else {
    cpf_=Pointer<ClosePairsFinder>(new GridClosePairsFinder());
  }
}



namespace {
  ParticlesTemp get_rigid_bodies(Model *m,
                                 const ParticleIndexes &pis) {
    IMP::base::set<Particle*> rets;
    for (unsigned int i=0; i< pis.size(); ++i) {
      if (RigidMember::particle_is_instance(m, pis[i])) {
        rets.insert(RigidMember(m, pis[i]).get_rigid_body());
      }
    };
    return ParticlesTemp(rets.begin(), rets.end());
  }

  /*ParticlesTemp get_non_rigid(SingletonContainer *sc) {
    ParticlesTemp ret;
    IMP_FOREACH_SINGLETON(sc, {
        if (!RigidMember::particle_is_instance(_1)) {
          ret.push_back(_1);
        }
      });
    return ret;
    }*/
  typedef IMP::base::map<Particle*, ParticleIndexes> RBM;
  void divvy_up_particles(const ParticlesTemp &ps,
                          ParticlesTemp &out,
                          RBM &members) {
    IMP_IF_CHECK(base::USAGE) {
      base::set<Particle*> ups(ps.begin(), ps.end());
      IMP_USAGE_CHECK(ups.size()==ps.size(),
                      "Duplicate particles in input: "
                      << ups.size() << "!= " << ps.size());
    }
    for (unsigned int i=0; i< ps.size(); ++i) {
      if (RigidMember::particle_is_instance(ps[i])) {
        RigidBody rb=RigidMember(ps[i]).get_rigid_body();
        if (members.find(rb)  == members.end()) {
          out.push_back(rb);
        }
        members[rb].push_back(ps[i]->get_index());
      } else {
        out.push_back(ps[i]);
      }
    }
    IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
      ParticlesTemp check_out=out;
      std::sort(check_out.begin(), check_out.end());
      check_out.erase(std::unique(check_out.begin(), check_out.end()),
                      check_out.end());
      IMP_INTERNAL_CHECK(check_out.size() == out.size(), "Values added twice: "
                         << check_out.size() << " vs " << out.size()
                         << ": " << out << " vs " << check_out);
    }
    /*std::cout << "Found " << members.size() << " rigid bodies and "
              << insc->get_number_of_particles()
              << " total particles" << std::endl;*/
  }

  void check_particles(const ParticlesTemp &ps) {
    IMP_IF_CHECK(base::USAGE) {
      for (ParticlesTemp::const_iterator it= ps.begin();
           it != ps.end(); ++it) {
        if (RigidBody::particle_is_instance(*it)
            && !(*it)->has_attribute(XYZR::get_radius_key())) {
          IMP_WARN("Particle " << (*it)->get_name() << " is a rigid body "
                   << "but does not have a radius. "
                   << "Collision detection is unlikely to work."
                   << std::endl);
        }
      }
    }
  }
}


ParticlePairsTemp RigidClosePairsFinder
::get_close_pairs(const ParticlesTemp &pa,
                  const ParticlesTemp &pb) const {
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE( "Rigid add_close_pairs called with "
          << pa.size() << " and "
          << pb.size()
          << " and distance " << get_distance()
          << std::endl);
  check_particles(pa);
  check_particles(pb);
  IMP::base::map<Particle*, ParticleIndexes> ma, mb;
  ParticlesTemp fa, fb;
  divvy_up_particles(pa, fa, ma);
  divvy_up_particles(pb, fb, mb);
  ParticlePairsTemp ppt= cpf_->get_close_pairs(fa,fb);
  ParticlePairsTemp ret;
  for (ParticlePairsTemp::const_iterator
         it= ppt.begin();
       it != ppt.end(); ++it) {
    // skip within one rigid body
    if (it->get(0) == it->get(1)) continue;
    ParticleIndexes ps0, ps1;
    if (ma.find(it->get(0)) != ma.end()) {
      ps0= ma.find(it->get(0))->second;
    }
    if (mb.find(it->get(1)) != mb.end()) {
      ps1= mb.find(it->get(1))->second;
    }
    if (ps0.empty() && ps1.empty()) {
      ret.push_back(*it);
    } else {
      ParticlePairsTemp c=get_close_pairs(it->get(0), it->get(1),
                                          ps0, ps1);
      ret.insert(ret.end(),
                 c.begin(), c.end());
    }
  }
  return ret;
}

ParticlePairsTemp RigidClosePairsFinder
::get_close_pairs(const ParticlesTemp &pa) const {
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE( "Adding close pairs from "
          << pa.size() << " particles." << std::endl);
  check_particles(pa);
  IMP::base::map<Particle*, ParticleIndexes> m;
  ParticlesTemp fa;
  divvy_up_particles(pa, fa, m);
  ParticlePairsTemp ppt= cpf_->get_close_pairs(fa);
  IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
    for (unsigned int i=0; i< ppt.size(); ++i) {
      IMP_INTERNAL_CHECK(ppt[i][0] != ppt[i][1],
                         "Pair of one returned: " << ppt[i][0]->get_name());
    }
  }
  ParticlePairsTemp ret;
  for (ParticlePairsTemp::const_iterator it= ppt.begin();
       it != ppt.end(); ++it) {
    ParticleIndexes ps0, ps1;
    IMP_LOG_VERBOSE( "Processing close pair " << *it << std::endl);
    if (m.find(it->get(0)) != m.end()) {
      ps0= m.find(it->get(0))->second;
    }
    if (m.find(it->get(1)) != m.end()) {
      ps1= m.find(it->get(1))->second;
    }
    if (ps0.empty() && ps1.empty()) {
      ret.push_back(*it);
    } else {
      ParticlePairsTemp c=get_close_pairs(it->get(0), it->get(1),
                                          ps0, ps1);
      ret.insert(ret.end(), c.begin(), c.end());
    }
  }
  return ret;
}


ParticlePairsTemp
RigidClosePairsFinder::get_close_pairs(Particle *a,
                                       Particle *b,
                                       const ParticleIndexes &ma,
                                       const ParticleIndexes &mb) const {
  IMP_INTERNAL_CHECK(a!= b, "Can't pass equal particles");
  internal::RigidBodyHierarchy *da=nullptr, *db=nullptr;
  ParticlePairsTemp out;
  if (ma.size()>0) {
    da= internal::get_rigid_body_hierarchy(RigidBody(a), ma, k_);
    IMP_INTERNAL_CHECK(da, "No hierarchy gotten");
  }
  if (mb.size()>0) {
    db= internal::get_rigid_body_hierarchy(RigidBody(b), mb, k_);
    IMP_INTERNAL_CHECK(db, "No hierarchy gotten");
  }
  /*IMP_INTERNAL_CHECK(RigidBody::particle_is_instance(a)==(da!=nullptr),
                     "Rigid body does not imply hierarchy");
  IMP_INTERNAL_CHECK(RigidBody::particle_is_instance(b)==(db!=nullptr),
  "Rigid body does not imply hierarchy");*/
  if (da && db) {
    out = internal::close_pairs(a->get_model(),
                                da,
                                db,
                                get_distance());
  } else if (da) {
    ParticlesTemp pt
      = internal::close_particles(a->get_model(), da,
                                  XYZR(b), get_distance());
    out.resize(pt.size());
    for (unsigned int i=0; i< pt.size(); ++i) {
      out[i]= ParticlePair(pt[i], b);
    }
  } else if (db) {
    ParticlesTemp pt
      = internal::close_particles(a->get_model(), db,
                                  XYZR(a), get_distance());
    out.resize(pt.size());
    for (unsigned int i=0; i< pt.size(); ++i) {
      out[i]= ParticlePair(a, pt[i]);
    }
  }
  IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
    for (unsigned int i=0; i< out.size(); ++i) {
      if (RigidMember::particle_is_instance(out[i][0])
          && RigidMember::particle_is_instance(out[i][1])) {
        IMP_INTERNAL_CHECK(RigidMember(out[i][0]).get_rigid_body()
                           != RigidMember(out[i][1]).get_rigid_body(),
                           "Particles from same rigid body when processing "
                           << a->get_name() << " and " << b->get_name()
                           << " with " << da << " and " << db << std::endl);
      }
    }
  }
  return out;
}



IntPairs RigidClosePairsFinder
::get_close_pairs(const algebra::BoundingBox3Ds &bas,
                  const algebra::BoundingBox3Ds &bbs) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  cpf_->set_distance(get_distance());
  return cpf_->get_close_pairs(bas, bbs);
}

IntPairs RigidClosePairsFinder
::get_close_pairs(const algebra::BoundingBox3Ds &bas) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  cpf_->set_distance(get_distance());
  return cpf_->get_close_pairs(bas);
}

/*
std::pair<algebra::Sphere3D, algebra::Sphere3D>
RigidClosePairsFinder::get_close_sphere_pair(Particle *a, Particle *b) const {

}*/

void RigidClosePairsFinder::do_show(std::ostream &out) const {
  out << "distance " << get_distance() << std::endl;
}

ModelObjectsTemp
RigidClosePairsFinder::do_get_inputs(Model *m,
                                    const ParticleIndexes &pis) const {
  ModelObjectsTemp ret= IMP::get_particles(m, pis);
  ParticlesTemp rbs= get_rigid_bodies(m, pis);
  ret+=rbs;
  for (unsigned int i=0; i< rbs.size(); ++i) {
    ret+= RigidBody(rbs[i]).get_members();
  }
  if (get_number_of_pair_filters() >0) {
    for (PairFilterConstIterator it= pair_filters_begin();
         it != pair_filters_end(); ++it) {
      for (unsigned int i=0; i< ret.size(); ++i) {
        ret+= (*it)->get_inputs(m, pis);
      }
    }
  }
  return ret;
}

internal::MovedSingletonContainer*
RigidClosePairsFinder::get_moved_singleton_container(SingletonContainer *in,
                                                     double threshold) const {
  return
    new internal::RigidMovedSingletonContainer(in, threshold);
}


IMPCORE_END_NAMESPACE
