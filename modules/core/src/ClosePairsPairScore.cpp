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
#include <IMP/core/RigidClosePairsFinder.h>
#include <IMP/algebra/internal/MinimalSet.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

ClosePairsPairScore::ClosePairsPairScore(PairScore *f,
                                         Refiner *r,
                                         Float thre): r_(r), f_(f),
                                                      th_(thre){
  IMP_USAGE_CHECK(thre >= 0, "The threshold must be non-negative.");

  cpf_=new RigidClosePairsFinder();
  ca_= new internal::CoreListSingletonContainer();
  cb_= new internal::CoreListSingletonContainer();
}


KClosePairsPairScore::KClosePairsPairScore(PairScore *f,
                                           Refiner *r,
                                           int k): r_(r), f_(f),
                                                   k_(k)
{
  last_distance_=1;
  cpf_= new RigidClosePairsFinder();
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
                        ParticlePairsTemp &pairs) {
    ParticlesTemp pa= expand(p[0], r);
    ParticlesTemp pb= expand(p[1], r);
    ca->set_particles(pa);
    cb->set_particles(pb);
    cpf->set_distance(dist);
    pairs= cpf->get_close_pairs(ca, cb);
    /*for (unsigned int i=0; i< ppt.size(); ++i) {
      double d=get_distance(XYZR(ppt[i][0]), XYZR(ppt[i][1]));
      if (d < dist) {
        distances.push_back(d);
        pairs.push_back(ppt[i]);
      }
      }*/
  }
  double do_evaluate(const ParticlePairsTemp &ppt,
                     PairScore *ps,
                     DerivativeAccumulator *da) {
    double ret=0;
    for (unsigned int i=0; i< ppt.size(); ++i) {
      IMP_LOG(VERBOSE, "Evaluating on " << ppt[i] << std::endl);
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
  fill_close_pairs(cpf_, ca_, cb_, th_, p, r_, ppt);
  return do_evaluate(ppt, f_, da);
}


double KClosePairsPairScore::evaluate(const ParticlePair &p,
                                     DerivativeAccumulator *da) const {
  //double mr= std::max(max_radius(psa), max_radius(psb));
  ParticlePairsTemp ppt;
  Floats dists;
  double dist=last_distance_;
  do {
    IMP_LOG(VERBOSE, "Searching for close pairs "
            << dist << std::endl);
    fill_close_pairs(cpf_, ca_, cb_, dist, p, r_, ppt);
    dist*=2;
  } while (ppt.size() < static_cast<unsigned int>(k_));
  algebra::internal::MinimalSet<double, ParticlePair> ms(k_);
  for (unsigned int i=0; i< ppt.size(); ++i) {
    double d= get_distance(XYZR(ppt[i][0]), XYZR(ppt[i][1]));
    if (ms.can_insert(d)) {
      ms.insert(d, ppt[i]);
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
  IMP_INTERNAL_CHECK(retps.size()==static_cast<unsigned int>(k_),
                     "Found " << retps.size()
                     << " but expected " << k_);
  IMP_IF_CHECK(USAGE) {
    if (k_==1) {
      double distance= get_distance(XYZR(retps[0][0]),
                                    XYZR(retps[0][1]));
      for (unsigned int i=0; i< r_->get_number_of_refined(p[0]); ++i) {
        for (unsigned int j=0; j< r_->get_number_of_refined(p[1]); ++j) {
          double cdistance= get_distance(XYZR(r_->get_refined(p[0], i)),
                                         XYZR(r_->get_refined(p[1], j)));
          IMP_USAGE_CHECK(cdistance >= distance-.1, "Missed shortest distance."
                          << " Got " << distance << " but just found "
                          << cdistance);
        }
      }
    }
  }
  return do_evaluate(retps, f_, da);
}
namespace {
  ParticlesTemp do_get_input_particles(Particle *p,
                                       Refiner *r,
                                       PairScore *f,
                                       ClosePairsFinder *cpf,
                                   internal::CoreListSingletonContainer *ca) {
  ParticlesTemp ret;
  ParticlesTemp ea=expand(p, r);
  for (unsigned int i=0; i< ea.size(); ++i) {
    ParticlesTemp c= f->get_input_particles(ea[i]);
    ret.insert(ret.end(), c.begin(), c.end());
  }
  ret.push_back(p);
  ParticlesTemp rp= r->get_input_particles(p);
  ret.insert(ret.end(), rp.begin(), rp.end());
  ca->set_particles(ea);
  ParticlesTemp cpfr= cpf->get_input_particles(ca);
  ret.insert(ret.end(), cpfr.begin(), cpfr.end());
  return ret;
  }
}

ParticlesTemp ClosePairsPairScore
::get_input_particles(Particle *p) const {
  return do_get_input_particles(p, r_, f_, cpf_, ca_);
}

ContainersTemp ClosePairsPairScore
::get_input_containers(Particle *p) const {
  ContainersTemp ret= r_->get_input_containers(p);
  ParticlesTemp ea=expand(p, r_);
  for (unsigned int i=0; i< ea.size(); ++i) {
    ContainersTemp c= f_->get_input_containers(ea[i]);
    ret.insert(ret.end(), c.begin(), c.end());
  }
  ContainersTemp rp= r_->get_input_containers(p);
  ret.insert(ret.end(), rp.begin(), rp.end());
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
  return do_get_input_particles(p, r_, f_, cpf_, ca_);
}

ContainersTemp KClosePairsPairScore
::get_input_containers(Particle *p) const {
  ContainersTemp ret= r_->get_input_containers(p);
  ParticlesTemp ea=expand(p, r_);
  for (unsigned int i=0; i< ea.size(); ++i) {
    ContainersTemp c= f_->get_input_containers(ea[i]);
    ret.insert(ret.end(), c.begin(), c.end());
  }
  ContainersTemp rp= r_->get_input_containers(p);
  ret.insert(ret.end(), rp.begin(), rp.end());
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
