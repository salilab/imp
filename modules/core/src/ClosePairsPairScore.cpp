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
#include <IMP/core/internal/close_pairs_helpers.h>
#include <IMP/core/internal/CoreListSingletonContainer.h>
#include <IMP/algebra/internal/MinimalSet.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

ClosePairsPairScore::ClosePairsPairScore(PairScore *f,
                                         Refiner *r,
                                         Float thre): r_(r), f_(f),
                                                      th_(thre){
  IMP_USAGE_CHECK(thre >= 0, "The threshold must be non-negative.");

  cpf_=internal::default_cpf();
  ca_= new internal::CoreListSingletonContainer();
  cb_= new internal::CoreListSingletonContainer();
}


KClosePairsPairScore::KClosePairsPairScore(PairScore *f,
                                           Refiner *r,
                                           int k): r_(r), f_(f),
                                                   k_(k)
{
  last_distance_=1;
  cpf_=internal::default_cpf();
  ca_= new internal::CoreListSingletonContainer();
  cb_= new internal::CoreListSingletonContainer();
}



namespace {
  ParticlesTemp expand(Particle *p, Refiner *r) {
    if (r->get_can_refine(p)) {
      return r->get_refined(p);
    } else {
      return ParticlesTemp(1,p);
    }
  }
  void fill_close_pairs(ClosePairsFinder* cpf,
                        internal::CoreListSingletonContainer* ca,
                        internal::CoreListSingletonContainer* cb,
                        double dist,
                        const ParticlePair &p,
                        Refiner* r,
                        Floats &distances,
                        ParticlePairsTemp &pairs) {
    ParticlesTemp pa= expand(p[0], r);
    ParticlesTemp pb= expand(p[1], r);
    ca->set_particles(pa);
    cb->set_particles(pb);
    cpf->set_distance(dist);
    ParticlePairsTemp ppt= cpf->get_close_pairs(ca, cb);
    for (unsigned int i=0; i< ppt.size(); ++i) {
      double d=get_distance(XYZR(ppt[i][0]), XYZR(ppt[i][1]));
      if (d < dist) {
        distances.push_back(d);
        pairs.push_back(ppt[i]);
      }
    }
  }
  double do_evaluate(const ParticlePairsTemp &ppt,
                     PairScore *ps,
                     DerivativeAccumulator *da) {
    double ret=0;
    for (unsigned int i=0; i< ppt.size(); ++i) {
      ret+=ps->evaluate(ppt[i], da);
    }
    return ret;
  }
  double max_radius(const ParticlesTemp &ps) {
    double mr=0;
    for (unsigned int i=0; i< ps.size(); ++i) {
      mr = std::max(mr, XYZR(ps[i]).get_radius());
    }
    return mr;
  }
}

double ClosePairsPairScore::evaluate(const ParticlePair &p,
                                     DerivativeAccumulator *da) const
{
  ParticlePairsTemp ppt;
  Floats dists;
  fill_close_pairs(cpf_, ca_, cb_, th_, p, r_, dists, ppt);
  return do_evaluate(ppt, f_, da);
}


double KClosePairsPairScore::evaluate(const ParticlePair &p,
                                     DerivativeAccumulator *da) const {
  //double mr= std::max(max_radius(psa), max_radius(psb));
  ParticlePairsTemp ppt;
  Floats dists;
  double dist=last_distance_;
  do {
    fill_close_pairs(cpf_, ca_, cb_, dist, p, r_, dists, ppt);
    dist*=2;
  } while (ppt.size() < static_cast<unsigned int>(k_));
  algebra::internal::MinimalSet<double, ParticlePair> ms(k_);
  for (unsigned int i=0; i< ppt.size(); ++i) {
    if (ms.can_insert(dists[i])) {
      ms.insert(dists[i], ppt[i]);
    }
  }
  if (ppt.size() > static_cast<unsigned int>(k_*2)) {
    last_distance_= dist*.25;
  } else {
    last_distance_= dist*.5;
  }
  ParticlePairsTemp retps;
  for (unsigned int i=0; i < ms.size(); ++i) {
    retps.push_back(ms[i].second);
  }
  return do_evaluate(retps, f_, da);
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


ParticlesTemp KClosePairsPairScore
::get_input_particles(Particle *p) const {
  ParticlesTemp ret;
  ParticlesTemp ea=expand(p, r_);
  for (unsigned int i=0; i< ea.size(); ++i) {
    ParticlesTemp c= f_->get_input_particles(ea[i]);
    ret.insert(ret.end(), c.begin(), c.end());
  }
  return ret;
}

ContainersTemp KClosePairsPairScore
::get_input_containers(Particle *p) const {
  ContainersTemp ret= r_->get_input_containers(p);
  ParticlesTemp ea=expand(p, r_);
  for (unsigned int i=0; i< ea.size(); ++i) {
    ContainersTemp c= f_->get_input_containers(ea[i]);
    ret.insert(ret.end(), c.begin(), c.end());
  }
  return ret;
}


bool KClosePairsPairScore::get_is_changed(const ParticlePair &) const {
  return true;
}

void KClosePairsPairScore::do_show(std::ostream &out) const
{
  out << "function " << *f_;
  out << "\nrefiner " << *r_ << std::endl;
}


IMPCORE_END_NAMESPACE
