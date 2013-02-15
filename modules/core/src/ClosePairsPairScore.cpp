/**
 *  \file ClosePairsPairScore.cpp
 *  \brief Iteratively refine to find all close pairs in a tree.
 *
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/core/ClosePairsPairScore.h>

#include <IMP/exception.h>
#include <IMP/log.h>
#include <IMP/algebra/vector_search.h>
#include <IMP/core/internal/rigid_body_tree.h>
#include <IMP/core/internal/close_pairs_helpers.h>
#include <IMP/internal/InternalListSingletonContainer.h>
#include <IMP/core/RigidClosePairsFinder.h>
#include <IMP/algebra/internal/MinimalSet.h>
#include <IMP/internal/container_helpers.h>
#include <IMP/core/PairRestraint.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

ClosePairsPairScore::ClosePairsPairScore(PairScore *f,
                                         Refiner *r,
                                         Float thre):
  PairScore("ClosePairsPirScore%1%"),
  r_(r), f_(f),
  th_(thre){
  IMP_USAGE_CHECK(thre >= 0, "The threshold must be non-negative.");

  cpf_=new RigidClosePairsFinder();
}


KClosePairsPairScore::KClosePairsPairScore(PairScore *f,
                                           Refiner *r,
                                           int k):
  PairScore("KClosePairsPairScore %1%"),
  r_(r), f_(f),
  k_(k)
{
  IMP_USAGE_CHECK(k_ >0, "Scoring on 0 close pairs isn't very useful");
  last_distance_=1;
  cpf_= new RigidClosePairsFinder();
}



namespace {
  ParticleIndexes expand(Particle *p, Refiner *r) {
    if (r->get_can_refine(p)) {
      ParticleIndexes ret= IMP::internal::get_index(r->get_refined(p));
      IMP_IF_CHECK(USAGE) {
        base::set<ParticleIndex> uret(ret.begin(), ret.end());
        IMP_USAGE_CHECK(uret.size()==ret.size(),
                        "Duplicate particles in refined result: "
                        << uret.size() << " != " << ret.size());
      }
      return ret;
    } else {
      return IMP::internal::get_index(ParticlesTemp(1,p));
    }
  }
  void fill_close_pairs(ClosePairsFinder* cpf,
                        Model *m,
                        double dist,
                        const ParticleIndexes &pa,
                        const ParticleIndexes &pb,
                        ParticleIndexPairs &pairs) {
    cpf->set_distance(dist);
    pairs= cpf->get_close_pairs(m, pa, pb);
    /*for (unsigned int i=0; i< ppt.size(); ++i) {
      double d=get_distance(XYZR(ppt[i][0]), XYZR(ppt[i][1]));
      if (d < dist) {
        distances.push_back(d);
        pairs.push_back(ppt[i]);
      }
      }*/
  }
}


ParticleIndexPairs
ClosePairsPairScore::get_close_pairs(Model *m,
                                     const ParticleIndexPair &p) const {
  ParticleIndexPairs ppt;
  Floats dists;
  ParticleIndexes ps0= expand(m->get_particle(p[0]), r_);
  ParticleIndexes ps1= expand(m->get_particle(p[1]), r_);
  fill_close_pairs(cpf_, m, th_, ps0, ps1, ppt);
  return ppt;
}

double ClosePairsPairScore::evaluate_index(Model *m,
                                     const ParticleIndexPair &pp,
                                     DerivativeAccumulator *da) const
{
  IMP_OBJECT_LOG;
  ParticleIndexPairs cps=get_close_pairs(m, pp);
  return f_->evaluate_indexes(m, cps, da,
                              0, cps.size());
}


double ClosePairsPairScore::evaluate_if_good_index(Model *m,
                                             const ParticleIndexPair &pp,
                                             DerivativeAccumulator *da,
                                             double max) const
{
  IMP_OBJECT_LOG;
  ParticleIndexPairs cps=get_close_pairs(m, pp);
  return f_->evaluate_if_good_indexes(m, cps, da, max, 0, cps.size());
}


ParticleIndexPairs KClosePairsPairScore::
get_close_pairs(Model *m,
                const ParticleIndexPair &p) const {
  IMP_OBJECT_LOG;
  //double mr= std::max(max_radius(psa), max_radius(psb));
  ParticleIndexPairs ppt;
  ParticleIndexes ps0= expand(m->get_particle(p[0]), r_);
  ParticleIndexes ps1= expand(m->get_particle(p[1]), r_);

  if (ps0.size()+ps1.size()>50) {
  Floats dists;
  double dist=last_distance_;
  IMP_USAGE_CHECK(ps0.size() > 0, "Empty set of particles used for "
                  << p[0]);
  IMP_USAGE_CHECK(ps1.size() > 0, "Empty set of particles used for "
                  << p[1]);
  do {
    IMP_LOG_VERBOSE( "Searching for close pairs "
            << dist << std::endl);
    fill_close_pairs(cpf_, m, dist,ps0, ps1, ppt);
    dist*=2;
    IMP_INTERNAL_CHECK(dist < std::numeric_limits<double>::max(),
                       "Something is not working for find pairs");
  } while (ppt.size() < static_cast<unsigned int>(k_));
  algebra::internal::MinimalSet<double, ParticleIndexPair> ms(k_);
  for (unsigned int i=0; i< ppt.size(); ++i) {
    double d= algebra::get_distance(m->get_sphere(ppt[i][0]),
                                    m->get_sphere(ppt[i][1]));
    //std::cout << "Trying " << d << " " << ppt[i] << std::endl;
    ms.insert(d, ppt[i]);
  }
  if (ppt.size() > static_cast<unsigned int>(k_*2)) {
    last_distance_= dist*.25;
  } else {
    last_distance_= dist*.5;
  }
  last_distance_= std::max(1.0, last_distance_);
  ParticleIndexPairs retps;
  for (unsigned int i=0; i < ms.size(); ++i) {
    //std::cout << "Got " << ms[i].second << std::endl;
    retps.push_back(ms[i].second);
  }
  IMP_INTERNAL_CHECK(retps.size()==static_cast<unsigned int>(k_),
                     "Found " << retps.size()
                     << " but expected " << k_);
  IMP_IF_CHECK(USAGE) {
    if (k_==1) {
      double distance= get_distance(XYZR(m, retps[0][0]),
                                    XYZR(m, retps[0][1]));
      for (unsigned int i=0; i< ps0.size(); ++i) {
        for (unsigned int j=0; j< ps1.size(); ++j) {
          double cdistance= get_distance(XYZR(m, ps0[i]),
                                         XYZR(m, ps1[j]));
          // mark as used
          if (0) std::cout << distance << cdistance;
          IMP_USAGE_CHECK(cdistance >= distance-.1, "Missed shortest distance."
                          << " Got " << distance << " but just found "
                          << cdistance);
        }
      }
    }
  }
  return retps;
  } else {
    algebra::internal::MinimalSet<double, ParticleIndexPair> ms(k_);
    for (unsigned int i=0; i< ps0.size(); ++i) {
      algebra::Sphere3D c0= m->get_sphere(ps0[i]);
      for (unsigned int j=0; j< ps1.size(); ++j) {
        algebra::Sphere3D c1= m->get_sphere(ps1[j]);
        double d= get_distance(c0, c1);
        /*std::cout << "Trying " << d << " "
          <<  ParticlePair(ps0[i], ps1[j]) << std::endl;*/
        ms.insert(d, ParticleIndexPair(ps0[i], ps1[j]));
      }
    }
    ParticleIndexPairs retps;
    for (unsigned int i=0; i< ms.size(); ++i) {
      //std::cout << "Got " << ms[i].second << std::endl;
      retps.push_back(ms[i].second);
    }
    return retps;
  }
}

double KClosePairsPairScore::evaluate_index(Model *m,
                                     const ParticleIndexPair &pp,
                                     DerivativeAccumulator *da) const {
  IMP_OBJECT_LOG;
  ParticleIndexPairs cps=get_close_pairs(m, pp);
  return f_->evaluate_indexes(m, cps, da, 0, cps.size());
}

double KClosePairsPairScore::evaluate_if_good_index(Model *m,
                                              const ParticleIndexPair &pp,
                                              DerivativeAccumulator *da,
                                              double max) const {
  IMP_OBJECT_LOG;
  ParticleIndexPairs cps=get_close_pairs(m, pp);
  return f_->evaluate_if_good_indexes(m,
                                      cps, da, max, 0, cps.size());
}

namespace {
  ModelObjectsTemp real_get_inputs(Model *m,
                                 const ParticleIndexes &pis,
                                 Refiner *r,
                                 PairScore *f,
                                 ClosePairsFinder *cpf) {
    ModelObjectsTemp ret;
    ParticleIndexes allpis;
    for (unsigned int i=0; i< pis.size(); ++i) {
      Particle *p= m->get_particle(pis[i]);
      if (r->get_can_refine(p)) {
        allpis+= IMP::internal::get_index(r->get_refined(p));
      } else {
        allpis.push_back(pis[i]);
      }
    }
    ret+= f->get_inputs(m, allpis);
    ret+= r->get_inputs(m, pis);
    ret+=cpf->get_inputs(m, allpis);
    return ret;
  }
}

ModelObjectsTemp ClosePairsPairScore
::do_get_inputs(Model *m,
                const ParticleIndexes &pis) const {
  return real_get_inputs(m, pis, r_, f_, cpf_);
}


void ClosePairsPairScore::do_show(std::ostream &out) const {
  out << "function " << *f_;
  out << "\nrefiner " << *r_ << std::endl;
}


ModelObjectsTemp KClosePairsPairScore
::do_get_inputs(Model *m,
                 const ParticleIndexes &pis) const {
  return real_get_inputs(m, pis, r_, f_, cpf_);
}


void KClosePairsPairScore::do_show(std::ostream &out) const
{
  out << "function " << *f_;
  out << "\nrefiner " << *r_ << std::endl;
}


Restraints ClosePairsPairScore
::create_current_decomposition(Model *m,
                               const ParticleIndexPair &vt) const {
  ParticleIndexPairs ppt= get_close_pairs(m, vt);
  Restraints ret(ppt.size());
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]= new PairRestraint(f_,
                              IMP::internal::get_particle(m,
                                                          ppt[i]));
  }
  return ret;
}


Restraints KClosePairsPairScore
::create_current_decomposition(Model *m,
                               const ParticleIndexPair &vt) const {
  ParticleIndexPairs ppt= get_close_pairs(m, vt);
  Restraints ret(ppt.size());
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]= new PairRestraint(f_,
                              IMP::internal::get_particle(m,
                                                          ppt[i]));
  }
  return ret;
}


IMPCORE_END_NAMESPACE
