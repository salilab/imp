/**
 *  \file RigidClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/RigidClosePairsFinder.h"
#include "IMP/core/BoxSweepClosePairsFinder.h"
#include "IMP/core/GridClosePairsFinder.h"
#include "IMP/core/rigid_bodies.h"
#include "IMP/core/internal/rigid_body_tree.h"
#include "IMP/core/XYZ.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/SingletonContainer.h>
#include <IMP/algebra/eigen_analysis.h>
#include <IMP/core/internal/Grid3D.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE


RigidClosePairsFinder::RigidClosePairsFinder(Refiner *r):
  ClosePairsFinder("RigidCPF"),
  r_(r){
#ifdef IMP_USE_CGAL
  cpf_=Pointer<ClosePairsFinder>(new BoxSweepClosePairsFinder());
#else
  cpf_=Pointer<ClosePairsFinder>(new GridClosePairsFinder());
#endif
  k_= internal::get_rigid_body_hierarchy_key(r_);
}
RigidClosePairsFinder
::RigidClosePairsFinder(ClosePairsFinder *cpf, Refiner *r):
  ClosePairsFinder("RigidCPF"),
  cpf_(cpf),
  r_(r){
  k_= internal::get_rigid_body_hierarchy_key(r_);
}



namespace {
  void check_particles(SingletonContainer *sc) {
    IMP_IF_CHECK(USAGE) {
      for (SingletonContainer::ParticleIterator it= sc->particles_begin();
         it != sc->particles_end(); ++it) {
        if (RigidBody::particle_is_instance(*it)
            && !(*it)->has_attribute(XYZR::get_default_radius_key())) {
          IMP_WARN("Particle " << (*it)->get_name() << " is a rigid body "
                   << "but does not have a radius. "
                   << "Collision detection is unlikely to work.");
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
  ParticlePairsTemp ppt= cpf_->get_close_pairs(ca,cb);
  ParticlePairsTemp ret;
  for (ParticlePairsTemp::const_iterator
         it= ppt.begin();
       it != ppt.end(); ++it) {
    ParticlePairsTemp c=get_close_pairs(it->get(0), it->get(1));
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
  ParticlePairsTemp ppt= cpf_->get_close_pairs(c);
  ParticlePairsTemp ret;
  for (ParticlePairsTemp::const_iterator it= ppt.begin();
       it != ppt.end(); ++it) {
    ParticlePairsTemp c=get_close_pairs(it->get(0), it->get(1));
    ret.insert(ret.end(), c.begin(), c.end());
  }
  return ret;
}


namespace {
  struct AddToContainer {
    bool swap_;
    mutable ParticlePairsTemp &out_;
    AddToContainer(ParticlePairsTemp &out, bool swap=false): swap_(swap),
                                                             out_(out){}
    void operator()(Particle *a, Particle *b) const {
      if (swap_) {
        out_.push_back(ParticlePair(b,a));
      } else {
        out_.push_back(ParticlePair(a,b));
      }
    }
  };
}


ParticlePairsTemp RigidClosePairsFinder::get_close_pairs(Particle *a,
                                                         Particle *b) const {
  internal::RigidBodyHierarchy *da=NULL, *db=NULL;
  ParticlePairsTemp out;
  if (RigidBody::particle_is_instance(a)) {
    da= internal::get_rigid_body_hierarchy(RigidBody(a), r_, k_);
  }
  if (RigidBody::particle_is_instance(b)) {
    db= internal::get_rigid_body_hierarchy(RigidBody(b), r_, k_);
  }
  if (da && db) {
    internal::apply_to_nearby(da, db, get_distance(),
                              AddToContainer(out));
  } else if (da) {
    internal::apply_to_nearby<AddToContainer,false>(da, XYZR(b), get_distance(),
                                     AddToContainer(out));
  } else if (db) {
    internal::apply_to_nearby<AddToContainer, true>(db, XYZR(a), get_distance(),
                                    AddToContainer(out, true));
  } else {
    out.push_back(ParticlePair(a,b));
  }
  return out;
}

void RigidClosePairsFinder::show(std::ostream &out) const {
  out << "RigidClosePairsFinder\n";
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
    for (unsigned int i=0; i< sc->get_number_of_particles(); ++i) {
      if (RigidBody::particle_is_instance(sc->get_particle(i))) {
        ContainersTemp m= r->get_input_containers(sc->get_particle(i));
        ret.insert(ret.end(), m.begin(), m.end());
      }
    }
    IMP_LOG(VERBOSE, "Input containers are " << Containers(ret) << std::endl);
    return ret;
  }
}

ParticlesTemp
RigidClosePairsFinder::get_input_particles(SingletonContainer *sc) const {
  ParticlesTemp ret= fill_list(r_, sc);
  return ret;
}

ParticlesTemp
RigidClosePairsFinder::get_input_particles(SingletonContainer *a,
                                           SingletonContainer *b) const {
  ParticlesTemp ret0= fill_list(r_, a);
  ParticlesTemp ret1= fill_list(r_, b);
  ret0.insert(ret0.end(), ret1.begin(), ret1.end());
  return ret0;
}

internal::MovedSingletonContainer*
RigidClosePairsFinder::get_moved_singleton_container(SingletonContainer *in,
                                                 Model *m,
                                                 double threshold) const {
  return new internal::MovedSingletonContainerImpl<std::pair<algebra::Sphere3D,
    algebra::Rotation3D>,
    internal::SaveXYZRRotValues,
    internal::SaveMovedValues<internal::SaveXYZRRotValues>,
    internal::ListXYZRRotMovedParticles>(m, in, threshold);
}



ContainersTemp
RigidClosePairsFinder::get_input_containers(SingletonContainer *sc) const {
  ContainersTemp ret= fill_containers(r_, sc);
  return ret;
}

ContainersTemp
RigidClosePairsFinder::get_input_containers(SingletonContainer *a,
                                           SingletonContainer *b) const {
  ContainersTemp ret= fill_containers(r_, a);
  ContainersTemp retp= fill_containers(r_, b);
  ret.insert(ret.end(), retp.begin(), retp.end());
  return ret;
}


IMPCORE_END_NAMESPACE
