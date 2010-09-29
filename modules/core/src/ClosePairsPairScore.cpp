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
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

ClosePairsPairScore::ClosePairsPairScore(Refiner *r,
                                         PairScore *f,
                                         Float thre): r_(r), f_(f),
                                                       th_(thre){
  k_=0;
  IMP_USAGE_CHECK(thre >= 0, "The threshold must be non-negative.");
}


ClosePairsPairScore::ClosePairsPairScore(Refiner *r,
                                         PairScore *f,
                                         int k): r_(r), f_(f),
                                                 k_(k)
{
}


namespace {

  ParticlesTemp expand(Particle* a, Refiner *r) {
    ParticlesTemp ret;
    ret.push_back(a);
    ParticlesTemp queue;
    queue.push_back(a);
    while (!queue.empty()) {
      Particle *p=queue.back();
      queue.pop_back();
      if (r->get_can_refine(p)) {
        ParticlesTemp rr= r->get_refined(p);
        queue.insert(queue.end(), rr.begin(), rr.end());
        ret.insert(ret.end(), rr.begin(), rr.end());
      }
    }
    return ret;
  }
}

double ClosePairsPairScore::evaluate(const ParticlePair &p,
                                     DerivativeAccumulator *da) const
{
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
    f_->evaluate(ppt[i], da);
  }
  return ret;
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
