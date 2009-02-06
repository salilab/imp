/**
 *  \file ClosePairsPairScore.cpp
 *  \brief Iteratively refine to find all close pairs in a tree.
 *
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include <IMP/core/ClosePairsPairScore.h>

#include <IMP/exception.h>
#include <IMP/log.h>

#include <cmath>

IMPCORE_BEGIN_NAMESPACE

ClosePairsPairScore::ClosePairsPairScore(ParticleRefiner *r,
                                         PairScore *f,
                                         Float thre,
                                         FloatKey rk): r_(r), f_(f),
                                                       th_(thre), rk_(rk){
  IMP_check(thre >= 0, "The threshold must be non-negative.",
            ValueException);
  IMP_check(rk != FloatKey(), "The radius-key must be non-default. "
            << "This is primarily a matter of laziness.",
            ValueException);
}

Float ClosePairsPairScore::evaluate(Particle *a, Particle *b,
                                    DerivativeAccumulator *da) const
{
  std::vector<ParticlePair> queue;
  queue.push_back(ParticlePair(a,b));
  // ick, and it isn't really even used
  std::vector<std::pair<Particle*, Particles> > refined;
  double ret=0;
  do {
    ParticlePair pp= queue.back();
    queue.pop_back();
    XYZRDecorator d0(pp[0]), d1(pp[1]);
    if (distance(d0, d1) > th_) {
      IMP_LOG(VERBOSE, "Particles "
              << d0 << " and " << d1
              << " are too far apart to refine "
              << " > " << th_<< std::endl);
      continue;
    }
    // may want to just refined 1 is th_ is nonzero
    Particles ps0, ps1;
    if (!r_->get_can_refine(pp[0])
        && !r_->get_can_refine(pp[1])) {
      ret+= f_->evaluate(pp[0], pp[1], da);
    } else {
      if (r_->get_can_refine(pp[0])) {
        ps0= r_->get_refined(pp[0]);
        refined.push_back(std::make_pair(pp[0], ps0));
      } else {
        ps0.push_back(pp[0]);
      }
      if (r_->get_can_refine(pp[1])) {
        ps1= r_->get_refined(pp[1]);
        refined.push_back(std::make_pair(pp[1], ps1));
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

  for (unsigned int i=0; i< refined.size(); ++i) {
    r_->cleanup_refined(refined[i].first, refined[i].second);
  }
  return ret;
}

void ClosePairsPairScore::show(std::ostream &out) const
{
  out << "ClosePairsPairScore using ";
  f_->show(out);
  r_->show(out);
}

IMPCORE_END_NAMESPACE
