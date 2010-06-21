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

#include <cmath>

IMPCORE_BEGIN_NAMESPACE

ClosePairsPairScore::ClosePairsPairScore(Refiner *r,
                                         PairScore *f,
                                         Float thre,
                                         FloatKey rk): r_(r), f_(f),
                                                       th_(thre), rk_(rk){
  IMP_USAGE_CHECK(thre >= 0, "The threshold must be non-negative.");
  IMP_USAGE_CHECK(rk != FloatKey(), "The radius-key must be non-default. "
            << "This is primarily a matter of laziness.");
}

namespace {
  ParticlePairsTemp get_close_pairs(Particle *a, Particle *b, double threshold,
                                    Refiner *r) {
    ParticlePairsTemp queue;
    ParticlePairsTemp ret;
    queue.push_back(ParticlePair(a,b));
    do {
      ParticlePair pp= queue.back();
      queue.pop_back();
      XYZR d0(pp[0]), d1(pp[1]);
      if (get_distance(d0, d1) > threshold) {
        IMP_LOG(VERBOSE, "Particles "
                << d0 << " and " << d1
                << " are too far apart to refine "
                << " > " << threshold<< std::endl);
        continue;
      }
    // may want to just refined 1 is th_ is nonzero
    ParticlesTemp ps0, ps1;
    if (!r->get_can_refine(pp[0])
        && !r->get_can_refine(pp[1])) {
      ret.push_back(pp);
    } else {
      if (r->get_can_refine(pp[0])) {
        ps0= r->get_refined(pp[0]);
      } else {
        ps0.push_back(pp[0]);
      }
      if (r->get_can_refine(pp[1])) {
        ps1= r->get_refined(pp[1]);
      } else {
        ps1.push_back(pp[1]);
      }
      for (unsigned int i=0; i< ps0.size(); ++i) {
        for (unsigned int j=0; j< ps1.size(); ++j) {
          queue.push_back(ParticlePair(ps0[i], ps1[j]));
        }
      }
    }
  } while (!queue.empty());
    return ret;
  }
}

double ClosePairsPairScore::evaluate(const ParticlePair &p,
                                     DerivativeAccumulator *da) const
{
  ParticlePairsTemp ppt= get_close_pairs(p[0], p[1], th_, r_);
  double ret=0;
  for (unsigned int i=0; i< ppt.size(); ++i) {
    ret+= f_->evaluate(ppt[i], da);
  }
  return ret;
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
