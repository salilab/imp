/**
 *  \file RigidClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
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
  ParticlesTemp get_rigid_bodies(SingletonContainer *sc) {
    IMP::internal::Set<Particle*> rets;
    IMP_FOREACH_SINGLETON(sc, {
        if (RigidMember::particle_is_instance(_1)) {
          rets.insert(RigidMember(_1).get_rigid_body());
        }
      });
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
  void divvy_up_particles(SingletonContainer *sc,
                          Refiner *r,
                          internal::CoreListSingletonContainer *insc,
                          RBM &members) {
    if (r) {
      insc->set_particles(sc->get_particles());
      IMP_FOREACH_SINGLETON(sc, {
          if (RigidBody::particle_is_instance(_1)) {
            members[_1]= r->get_refined(_1);
          }
        });
    } else {
      ParticlesTemp simple;
      IMP_FOREACH_SINGLETON(sc, {
          if (RigidMember::particle_is_instance(_1)) {
            if (members.find(RigidMember(_1).get_rigid_body())
                == members.end()) {
              simple.push_back(RigidMember(_1).get_rigid_body());
            }
            members[RigidMember(_1).get_rigid_body()].push_back(_1);
          } else {
            simple.push_back(_1);
          }
        });
      insc->set_particles(simple);
    }
    std::cout << "Found " << members.size() << " rigid bodies and "
              << insc->get_number_of_particles()
              << " total particles" << std::endl;
  }

  void check_particles(SingletonContainer *sc) {
    IMP_IF_CHECK(USAGE) {
      for (SingletonContainer::ParticleIterator it= sc->particles_begin();
         it != sc->particles_end(); ++it) {
        if (RigidBody::particle_is_instance(*it)
            && !(*it)->has_attribute(XYZR::get_default_radius_key())) {
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
::get_close_pairs(SingletonContainer *ca,
                  SingletonContainer *cb) const {
  IMP_LOG(TERSE, "Rigid add_close_pairs called with "
          << ca->get_number_of_particles() << " and "
          << cb->get_number_of_particles() << std::endl);
  check_particles(ca);
  check_particles(cb);
  IMP_NEW(internal::CoreListSingletonContainer, fca, ());
  IMP_NEW(internal::CoreListSingletonContainer, fcb, ());
  IMP::internal::Map<Particle*, ParticlesTemp> ma, mb;
  divvy_up_particles(ca, r_, fca, ma);
  divvy_up_particles(cb, r_, fcb, mb);
  ParticlePairsTemp ppt= cpf_->get_close_pairs(fca,fcb);
  ParticlePairsTemp ret;
  for (ParticlePairsTemp::const_iterator
         it= ppt.begin();
       it != ppt.end(); ++it) {
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
::get_close_pairs(SingletonContainer *c) const {
  IMP_LOG(TERSE, "Adding close pairs from "
          << c->get_number_of_particles() << " particles." << std::endl);
  check_particles(c);
  IMP_NEW(internal::CoreListSingletonContainer, fc, ());
  IMP::internal::Map<Particle*, ParticlesTemp> m;
  divvy_up_particles(c, r_, fc, m);
  ParticlePairsTemp ppt= cpf_->get_close_pairs(fc);
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


namespace {
  struct AddToContainer {
    bool swap_;
    mutable ParticlePairsTemp &out_;
    IMP::internal::Set<Particle*> sa_, sb_;
    AddToContainer(ParticlePairsTemp &out,
                   const ParticlesTemp &psa,
                   const ParticlesTemp &psb,
                   bool swap=false): swap_(swap),
                                     out_(out),
                                     sa_(psa.begin(), psa.end()),
                                     sb_(psb.begin(), psb.end()){}
    void operator()(Particle *a, Particle *b) const {
      if (sa_.find(a) != sa_.end() && sb_.find(b) != sb_.end()) {
        if (swap_) {
          out_.push_back(ParticlePair(b,a));
        } else {
          out_.push_back(ParticlePair(a,b));
        }
      }
    }
  };
}


ParticlePairsTemp
RigidClosePairsFinder::get_close_pairs(Particle *a,
                                       Particle *b,
                                       const ParticlesTemp &ma,
                                       const ParticlesTemp &mb) const {
  internal::RigidBodyHierarchy *da=NULL, *db=NULL;
  ParticlePairsTemp out;
  if (ma.size()>0 && ma[0] != a) {
    da= internal::get_rigid_body_hierarchy(RigidBody(a), k_);
  }
  if (mb.size()>0 && mb[0] != b) {
    db= internal::get_rigid_body_hierarchy(RigidBody(b), k_);
  }
  if (da && db) {
    internal::apply_to_nearby(da, db, get_distance(),
                              AddToContainer(out, ma, mb));
  } else if (da) {
    internal::apply_to_nearby<AddToContainer,false>(da, XYZR(b), get_distance(),
                                                    AddToContainer(out,
                                                                   ma, mb));
  } else if (db) {
    internal::apply_to_nearby<AddToContainer, true>(db, XYZR(a), get_distance(),
                                             AddToContainer(out, ma, mb, true));
  } else {
    out.push_back(ParticlePair(a,b));
  }
  return out;
}



IntPairs RigidClosePairsFinder
::get_close_pairs(const algebra::BoundingBox3Ds &bas,
                  const algebra::BoundingBox3Ds &bbs) const {
  set_was_used(true);
  cpf_->set_distance(get_distance());
  return cpf_->get_close_pairs(bas, bbs);
}

IntPairs RigidClosePairsFinder
::get_close_pairs(const algebra::BoundingBox3Ds &bas) const {
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
  ParticlesTemp fill_list(Refiner *r, SingletonContainer *sc) {
    ParticlesTemp ret=sc->get_particles();
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

  ContainersTemp fill_containers(Refiner *r, SingletonContainer *sc) {
    ContainersTemp ret(1, sc);
    IMP_FOREACH_SINGLETON(sc, {
        if (RigidBody::particle_is_instance(_1)) {
          ContainersTemp m= r->get_input_containers(_1);
          ret.insert(ret.end(), m.begin(), m.end());
        }
      });
    IMP_LOG(VERBOSE, "Input containers are " << Containers(ret) << std::endl);
    return ret;
  }
}

ParticlesTemp
RigidClosePairsFinder::get_input_particles(SingletonContainer *sc) const {
  if (r_) {
    ParticlesTemp ret= fill_list(r_, sc);
    return ret;
  } else {
    ParticlesTemp ret(sc->particles_begin(), sc->particles_end());
    ParticlesTemp rbs= get_rigid_bodies(sc);
    ret.insert(ret.end(), rbs.begin(), rbs.end());
    return ret;
  }
}

ParticlesTemp
RigidClosePairsFinder::get_input_particles(SingletonContainer *a,
                                           SingletonContainer *b) const {
  if (r_) {
    ParticlesTemp ret0= fill_list(r_, a);
    ParticlesTemp ret1= fill_list(r_, b);
    ret0.insert(ret0.end(), ret1.begin(), ret1.end());
    return ret0;
  } else {
    ParticlesTemp ret(a->particles_begin(), a->particles_end());
    ParticlesTemp rbs= get_rigid_bodies(a);
    ret.insert(ret.end(), rbs.begin(), rbs.end());
    ParticlesTemp retb(b->particles_begin(), b->particles_end());
    ParticlesTemp rbsb= get_rigid_bodies(b);
    ret.insert(ret.end(), rbsb.begin(), rbsb.end());
    ret.insert(ret.end(), retb.begin(), retb.end());
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
RigidClosePairsFinder::get_input_containers(SingletonContainer *sc) const {
  if (r_) {
    ContainersTemp ret= fill_containers(r_, sc);
    return ret;
  } else {
    ParticlesTemp rbs= get_rigid_bodies(sc);
    ContainersTemp ret(rbs.begin(), rbs.end());
    ret.push_back(sc);
    return ret;
  }
}

ContainersTemp
RigidClosePairsFinder::get_input_containers(SingletonContainer *a,
                                           SingletonContainer *b) const {
  if (r_) {
    ContainersTemp ret= fill_containers(r_, a);
    ContainersTemp retp= fill_containers(r_, b);
    ret.insert(ret.end(), retp.begin(), retp.end());
    return ret;
  } else {
    ParticlesTemp rba= get_rigid_bodies(a);
    ParticlesTemp rbb= get_rigid_bodies(b);
    ContainersTemp ret(rba.begin(), rbb.end());
    ret.insert(ret.end(), rbb.begin(), rbb.end());
    ret.push_back(a);
    ret.push_back(b);
    return ret;
  }
}

IMPCORE_END_NAMESPACE
