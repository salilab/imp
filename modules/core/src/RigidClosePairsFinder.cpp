/**
 *  \file RigidClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/RigidClosePairsFinder.h"
#include "IMP/core/GridClosePairsFinder.h"
#include "IMP/core/rigid_bodies.h"
#include "IMP/core/internal/rigid_body_tree.h"
#include "IMP/core/XYZ.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/SingletonContainer.h>
#include <IMP/core/internal/CoreListSingletonContainer.h>
#include <IMP/macros.h>
#include <IMP/algebra/eigen_analysis.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

RigidClosePairsFinder::RigidClosePairsFinder(Refiner *r):
  ClosePairsFinder("RigidCPF"),
  r_(r){
  cpf_=Pointer<ClosePairsFinder>(new GridClosePairsFinder());
  k_= internal::get_rigid_body_hierarchy_key();
}
RigidClosePairsFinder
::RigidClosePairsFinder(ClosePairsFinder *cpf, Refiner *r):
  ClosePairsFinder("RigidCPF"),
  cpf_(cpf),
  r_(r){
  k_= internal::get_rigid_body_hierarchy_key();
}


RigidClosePairsFinder
::RigidClosePairsFinder(ClosePairsFinder *cpf):
  ClosePairsFinder("RigidCPF") {
  k_= internal::get_rigid_body_hierarchy_key();
  if (cpf) {
    cpf_=cpf;
  } else {
    cpf_=Pointer<ClosePairsFinder>(new GridClosePairsFinder());
  }
}



namespace {
  ParticlesTemp get_rigid_bodies(const ParticlesTemp &sc) {
    IMP::internal::Set<Particle*> rets;
    for (unsigned int i=0; i< sc.size(); ++i) {
      if (RigidMember::particle_is_instance(sc[i])) {
        rets.insert(RigidMember(sc[i]).get_rigid_body());
      }
    };
    return ParticlesTemp(rets.begin(), rets.end());
  }

  ParticlesTemp get_non_rigid(SingletonContainer *sc) {
    ParticlesTemp ret;
    IMP_FOREACH_SINGLETON(sc, {
        if (!RigidMember::particle_is_instance(_1)) {
          ret.push_back(_1);
        }
      });
    return ret;
  }
  typedef IMP::internal::Map<Particle*, ParticlesTemp> RBM;
  void divvy_up_particles(const ParticlesTemp &ps,
                          Refiner *r,
                          ParticlesTemp &out,
                          RBM &members) {
    if (r) {
      out=ps;
      for (unsigned int i=0; i< ps.size(); ++i) {
        if (RigidBody::particle_is_instance(ps[i])) {
            members[ps[i]]= r->get_refined(ps[i]);
          }
      }
    } else {
      for (unsigned int i=0; i< ps.size(); ++i) {
        if (RigidMember::particle_is_instance(ps[i])) {
          if (members.find(RigidMember(ps[i]).get_rigid_body())
              == members.end()) {
            out.push_back(RigidMember(ps[i]).get_rigid_body());
          }
          members[RigidMember(ps[i]).get_rigid_body()].push_back(ps[i]);
        } else {
          out.push_back(ps[i]);
        }
      }
    }
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      ParticlesTemp check_out=out;
      std::sort(check_out.begin(), check_out.end());
      check_out.erase(std::unique(check_out.begin(), check_out.end()),
                      check_out.end());
      IMP_INTERNAL_CHECK(check_out.size() == out.size(), "Values added twice");
    }
    /*std::cout << "Found " << members.size() << " rigid bodies and "
              << insc->get_number_of_particles()
              << " total particles" << std::endl;*/
  }

  void check_particles(const ParticlesTemp &ps) {
    IMP_IF_CHECK(USAGE) {
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
  IMP_LOG(TERSE, "Rigid add_close_pairs called with "
          << pa.size() << " and "
          << pb.size() << std::endl);
  check_particles(pa);
  check_particles(pb);
  IMP::internal::Map<Particle*, ParticlesTemp> ma, mb;
  ParticlesTemp fa, fb;
  divvy_up_particles(pa, r_, fa, ma);
  divvy_up_particles(pb, r_, fb, mb);
  ParticlePairsTemp ppt= cpf_->get_close_pairs(fa,fb);
  ParticlePairsTemp ret;
  for (ParticlePairsTemp::const_iterator
         it= ppt.begin();
       it != ppt.end(); ++it) {
    // skip within one rigid body
    if (it->get(0) == it->get(1)) continue;
    ParticlesTemp ps0, ps1;
    if (ma.find(it->get(0)) != ma.end()) {
      ps0= ma.find(it->get(0))->second;
    } else {
      ps0= ParticlesTemp(1, it->get(0));
    }
    if (mb.find(it->get(1)) != mb.end()) {
      ps1= mb.find(it->get(1))->second;
    } else {
      ps1= ParticlesTemp(1, it->get(1));
    }
    ParticlePairsTemp c=get_close_pairs(it->get(0), it->get(1),
                                        ps0, ps1);
    ret.insert(ret.end(),
               c.begin(), c.end());
  }
  return ret;
}

ParticlePairsTemp RigidClosePairsFinder
::get_close_pairs(const ParticlesTemp &pa) const {
  IMP_OBJECT_LOG;
  IMP_LOG(TERSE, "Adding close pairs from "
          << pa.size() << " particles." << std::endl);
  check_particles(pa);
  IMP::internal::Map<Particle*, ParticlesTemp> m;
  ParticlesTemp fa;
  divvy_up_particles(pa, r_, fa, m);
  ParticlePairsTemp ppt= cpf_->get_close_pairs(fa);
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    for (unsigned int i=0; i< ppt.size(); ++i) {
      IMP_INTERNAL_CHECK(ppt[i][0] != ppt[i][1],
                         "Pair of one returned: " << ppt[i][0]);
    }
  }
  ParticlePairsTemp ret;
  for (ParticlePairsTemp::const_iterator it= ppt.begin();
       it != ppt.end(); ++it) {
    ParticlesTemp ps0, ps1;
    if (m.find(it->get(0)) != m.end()) {
      ps0= m.find(it->get(0))->second;
    } else {
      ps0= ParticlesTemp(1, it->get(0));
    }
    if (m.find(it->get(1)) != m.end()) {
      ps1= m.find(it->get(1))->second;
    } else {
      ps1= ParticlesTemp(1, it->get(1));
    }
    ParticlePairsTemp c=get_close_pairs(it->get(0), it->get(1),
                                        ps0, ps1);
    ret.insert(ret.end(), c.begin(), c.end());
  }
  return ret;
}


ParticlePairsTemp
RigidClosePairsFinder::get_close_pairs(Particle *a,
                                       Particle *b,
                                       const ParticlesTemp &ma,
                                       const ParticlesTemp &mb) const {
  IMP_INTERNAL_CHECK(a!= b, "Can't pass equal particles");
  internal::RigidBodyHierarchy *da=NULL, *db=NULL;
  ParticlePairsTemp out;
  if (ma.size()>0 && ma[0] != a) {
    da= internal::get_rigid_body_hierarchy(RigidBody(a), k_);
  }
  if (mb.size()>0 && mb[0] != b) {
    db= internal::get_rigid_body_hierarchy(RigidBody(b), k_);
  }
  if (da && db) {
    out = internal::close_pairs(da, IMP::internal::Set<Particle*>(ma.begin(),
                                                                  ma.end()),
                              db, IMP::internal::Set<Particle*>(mb.begin(),
                                                                mb.end()),
                              get_distance());
  } else if (da) {
    ParticlesTemp pt
      = internal::close_particles(da, IMP::internal::Set<Particle*>(ma.begin(),
                                                                    ma.end()),
                                  XYZR(b), get_distance());
    out.resize(pt.size());
    for (unsigned int i=0; i< pt.size(); ++i) {
      out[i]= ParticlePair(pt[i], b);
    }
  } else if (db) {
    ParticlesTemp pt
      = internal::close_particles(db, IMP::internal::Set<Particle*>(mb.begin(),
                                                                    mb.end()),
                                                XYZR(a), get_distance());
    out.resize(pt.size());
    for (unsigned int i=0; i< pt.size(); ++i) {
      out[i]= ParticlePair(a, pt[i]);
    }
  } else {
    out.push_back(ParticlePair(a,b));
  }
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
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

namespace {
  ParticlesTemp fill_list(Refiner *r,  ParticlesTemp ret) {
    ParticlesTemp ret2;
    for (unsigned int i=0; i< ret.size(); ++i) {
      if (RigidBody::particle_is_instance(ret[i])) {
        ParticlesTemp m= r->get_input_particles(ret[i]);
        ParticlesTemp rm= r->get_refined(ret[i]);
        m.insert(m.end(), rm.begin(), rm.end());
        IMP_LOG(VERBOSE, "Used particles for " << ret[i]->get_name() << " are "
                << Particles(m) << std::endl);
        ret2.insert(ret2.end(), m.begin(), m.end());
      } else {
        ret2.push_back(ret[i]);
      }
    }
    ret.insert(ret.end(), ret2.begin(), ret2.end());
   IMP_LOG(VERBOSE, "Input particles are " << Particles(ret) << std::endl);
    return ret;
  }

  ContainersTemp fill_containers(Refiner *r, const ParticlesTemp &pa) {
    ContainersTemp ret;
    for (unsigned int i=0; i< pa.size(); ++i) {
        if (RigidBody::particle_is_instance(pa[i])) {
          ContainersTemp m= r->get_input_containers(pa[i]);
          ret.insert(ret.end(), m.begin(), m.end());
        }
    };
    IMP_LOG(VERBOSE, "Input containers are " << Containers(ret) << std::endl);
    return ret;
  }
}

ParticlesTemp
RigidClosePairsFinder::get_input_particles(const ParticlesTemp &pa) const {
  if (r_) {
    ParticlesTemp ret= fill_list(r_, pa);
    return ret;
  } else {
    ParticlesTemp ret= pa;
    ParticlesTemp rbs= get_rigid_bodies(pa);
    ret.insert(ret.end(), rbs.begin(), rbs.end());
    for (unsigned int i=0; i< rbs.size(); ++i) {
      ParticlesTemp rm= RigidBody(rbs[i]).get_members();
      ret.insert(ret.end(), rm.begin(), rm.end());
    }
    return ret;
  }
}

internal::MovedSingletonContainer*
RigidClosePairsFinder::get_moved_singleton_container(SingletonContainer *in,
                                                 Model *m,
                                                     double threshold) const {
  if (r_) {
    return
      new internal::MovedSingletonContainerImpl<std::pair<algebra::SphereD<3>,
                                                          algebra::Rotation3D>,
                                                internal::SaveXYZRRotValues,
             internal::SaveMovedValues<internal::SaveXYZRRotValues>,
                        internal::ListXYZRRotMovedParticles>(m, in, threshold);
  } else {
    // make more efficient later
    return ClosePairsFinder::get_moved_singleton_container(in, m, threshold);
  }
}



ContainersTemp
RigidClosePairsFinder::get_input_containers(const ParticlesTemp &pa) const {
  if (r_) {
    ContainersTemp ret= fill_containers(r_, pa);
    return ret;
  } else {
    ParticlesTemp rbs= get_rigid_bodies(pa);
    ContainersTemp ret(rbs.begin(), rbs.end());
    return ret;
  }
}

IMPCORE_END_NAMESPACE
