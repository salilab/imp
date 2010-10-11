/**
 *  \file ClosePairsPairScore.cpp
 *  \brief Iteratively refine to find all close pairs in a tree.
 *
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP/core/ClosePairsPairScore.h>

#include <IMP/exception.h>
#include <IMP/log.h>
#include <IMP/algebra/vector_search.h>
#include <IMP/core/internal/rigid_body_tree.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

ClosePairsPairScore::ClosePairsPairScore(PairScore *f,
                                         Refiner *r,
                                         Float thre): r_(r), f_(f),
                                                      th_(thre), k_(0){
  k_=0;
  IMP_USAGE_CHECK(thre >= 0, "The threshold must be non-negative.");
}


ClosePairsPairScore::ClosePairsPairScore(PairScore *f,
                                         Refiner *r,
                                         int k): r_(r), f_(f),
                                                 k_(k)
{
}



namespace {
  ParticlePair get_closest_pair(Particle *a, Particle *b,
                                Refiner* ra, Refiner *rb, ObjectKey ka,
                                ObjectKey kb) {
    internal::RigidBodyHierarchy *da=NULL, *db=NULL;
    if (RigidBody::particle_is_instance(a)) {
      da= internal::get_rigid_body_hierarchy(RigidBody(a), ka);
    }
    if (RigidBody::particle_is_instance(b)) {
      db= internal::get_rigid_body_hierarchy(RigidBody(b), kb);
    }
    if (!da && !db) {
      return ParticlePair(a,b);
    } else if (!da) {
      ParticlesTemp psb=rb->get_refined(b);
      IMP::internal::Set<Particle*> pb(psb.begin(), psb.end());
      Particle *p= internal::closest_particle(db, pb,
                                              XYZR(a));
      return ParticlePair(a,p);
    } else if (!db) {
      ParticlesTemp psa=ra->get_refined(a);
      IMP::internal::Set<Particle*> pa(psa.begin(), psa.end());
      Particle *p= internal::closest_particle(da, pa, XYZR(b));
      return ParticlePair(p,b);
    } else {
      ParticlesTemp psa=ra->get_refined(a);
      IMP::internal::Set<Particle*> pa(psa.begin(), psa.end());
      ParticlesTemp psb=rb->get_refined(b);
      IMP::internal::Set<Particle*> pb(psb.begin(), psb.end());
      ParticlePair pp= internal::closest_pair(da, pa,
                                              db, pb);
      return pp;
    }
  }
  ParticlesTemp expand(Particle *p, Refiner *r) {
    if (r->get_can_refine(p)) {
      return r->get_refined(p);
    } else {
      return ParticlesTemp(1,p);
    }
  }
}

double ClosePairsPairScore::evaluate(const ParticlePair &p,
                                     DerivativeAccumulator *da) const
{
  if (k_==1 && RigidBody::particle_is_instance(p[0])
      && RigidBody::particle_is_instance(p[1])) {
    ParticlePair pp= get_closest_pair(p[0], p[1], r_, r_,
                                      internal::get_rigid_body_hierarchy_key(),
                                      internal::get_rigid_body_hierarchy_key());
    IMP_LOG(VERBOSE, "Closest rigid body pair for bodies "
            << p[0]->get_name() << " and " << p[1]->get_name()
            << " is " << pp[0]->get_name() << " and " << pp[1]->get_name()
            << " with coordinates " << XYZ(pp[0]) << " and " << XYZ(pp[1])
            << std::endl);
    return f_->evaluate(pp, da);
  } else {
    XYZsTemp xyzs0(expand(p[0], r_));
    XYZsTemp xyzs1(expand(p[1], r_));
    algebra::NearestNeighborD<3> nn(xyzs0.begin(),
                                    xyzs0.end());
    ParticlePairsTemp ppt;
    if (k_>0) {
      algebra::internal::MinimalSet<double, ParticlePair> ms(k_);
      for (unsigned int i=0; i< xyzs1.size(); ++i) {
        Ints is= nn.get_nearest_neighbors(xyzs1[i].get_coordinates(),
                                          k_);
        for (unsigned int j=0; j< is.size(); ++j) {
          double d= get_distance(xyzs0[is[j]], xyzs1[i]);
          if (ms.can_insert(d)) {
          ms.insert(d, ParticlePair(xyzs0[is[j]], xyzs1[i]));
          }
        }
      }
      for (unsigned int i=0; i< ms.size(); ++i) {
        ppt.push_back(ms[i].second);
      }
    } else {
      for (unsigned int i=0; i< xyzs1.size(); ++i) {
        Ints is= nn.get_in_ball(xyzs1[i].get_coordinates(),
                                th_);
        for (unsigned int j=0; j< is.size(); ++j) {
          ppt.push_back(ParticlePair(xyzs0[is[j]], xyzs1[i]));
        }
      }
    }
    double ret=0;
    for (unsigned int i=0; i< ppt.size(); ++i) {
      ret+=f_->evaluate(ppt[i], da);
    }
    return ret;
  }
}


ParticlesTemp ClosePairsPairScore
::get_input_particles(Particle *p) const {
  ParticlesTemp ret;
  ParticlesTemp ea=expand(p, r_);
  for (unsigned int i=0; i< ea.size(); ++i) {
    ParticlesTemp c= f_->get_input_particles(ea[i]);
    ret.insert(ret.end(), c.begin(), c.end());
  }
  return ret;
}

ContainersTemp ClosePairsPairScore
::get_input_containers(Particle *p) const {
  ContainersTemp ret= r_->get_input_containers(p);
  ParticlesTemp ea=expand(p, r_);
  for (unsigned int i=0; i< ea.size(); ++i) {
    ContainersTemp c= f_->get_input_containers(ea[i]);
    ret.insert(ret.end(), c.begin(), c.end());
  }
  return ret;
}


bool ClosePairsPairScore::get_is_changed(const ParticlePair &) const {
  return true;
}

void ClosePairsPairScore::do_show(std::ostream &out) const
{
  out << "function " << *f_;
  out << "\nrefiner " << *r_ << std::endl;
}

IMPCORE_END_NAMESPACE
