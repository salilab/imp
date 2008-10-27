/**
 *  \file LowestRefinedPairScore.cpp
 *  \brief Lowest particles at most refined with a ParticleLowestr.
 *
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include <IMP/misc/LowestRefinedPairScore.h>

#include <IMP/core/bond_decorators.h>
#include <IMP/core/XYZDecorator.h>
#include <IMP/internal/constants.h>

#include <cmath>

IMPMISC_BEGIN_NAMESPACE

LowestRefinedPairScore::LowestRefinedPairScore(ParticleRefiner *r,
                                               PairScore *f): r_(r), f_(f) {}

Float LowestRefinedPairScore::evaluate(Particle *a, Particle *b,
                                    DerivativeAccumulator *da) const
{
  Particle* p[2]={a,b};
  Particles ps[2];
  for (unsigned int i=0; i< 2; ++i) {
    if (r_->get_can_refine(p[i])) {
      ps[i]= r_->get_refined(p[i]);
    } else {
      ps[i].push_back(p[i]);
    }
    IMP_LOG(VERBOSE, "Refining " << p[i]->get_index()
            << " resulted in " << ps[i].size() << " particles"
            << std::endl);
  }

  Float ret=std::numeric_limits<Float>::max();
  ParticlePair lowest;
  for (unsigned int i=0; i< ps[0].size(); ++i) {
    for (unsigned int j=0; j< ps[1].size(); ++j) {
      Float v= f_->evaluate(ps[0][i], ps[1][j], NULL);
      if (v < ret) {
        ret=v;
        lowest= ParticlePair(ps[0][i], ps[1][j]);
      }
    }
  }

  if (da) {
    f_->evaluate(lowest.first, lowest.second, da);
  }

  for (unsigned int i=0; i< 2; ++i) {
    if (ps[i].size() != 1 || (ps[i].size()==1 && ps[i][0] != p[i])) {
      IMP_LOG(VERBOSE, "Refining " << p[i]->get_index()
              << " resulted in " << ps[i].size() << " particles"
              << std::endl);
      r_->cleanup_refined(p[i], ps[i], da);
    }
  }
  return ret;
}

void LowestRefinedPairScore::show(std::ostream &out) const
{
  out << "LowestRefinedPairScore using ";
  f_->show(out);
  r_->show(out);
}

IMPMISC_END_NAMESPACE
