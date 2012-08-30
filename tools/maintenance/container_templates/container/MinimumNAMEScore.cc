/**
 *  \file MinimumCLASSNAMEScore.cpp  \brief Define CLASSNAMEScore
 *
 *  BLURB
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/container/MinimumCLASSNAMEScore.h>
#include <IMP/core/CLASSNAMERestraint.h>
#include "IMP/algebra/internal/MinimalSet.h"

IMPCONTAINER_BEGIN_NAMESPACE

MinimumCLASSNAMEScore::MinimumCLASSNAMEScore(const CLASSNAMEScoresTemp &scores,
                                           unsigned int n,
                                           std::string name):
  CLASSNAMEScore(name),
  scores_(scores.begin(), scores.end()),
  n_(n)
{
}


namespace {
  typedef algebra::internal::MinimalSet<double,
          CLASSNAMEScore*, std::less<double> >
  MinimumCLASSNAMEScoreMS;
  template <class It>
  MinimumCLASSNAMEScoreMS
  find_minimal_set_MinimumCLASSNAMEScore(It b, It e, ARGUMENTTYPE v,
                                          unsigned int n) {
    IMP_LOG(TERSE, "Finding Minimum " << n << " of "
            << std::distance(b,e) << std::endl);
    MinimumCLASSNAMEScoreMS bestn(n);
    for (It it= b; it != e; ++it) {
      double score= (*it)->evaluate(v, nullptr);
      bestn.insert(score, *it);
    }
    return bestn;
  }
}

double MinimumCLASSNAMEScore::evaluate(ARGUMENTTYPE v,
                                      DerivativeAccumulator *da) const {
  MinimumCLASSNAMEScoreMS bestn
    = find_minimal_set_MinimumCLASSNAMEScore(scores_.begin(),
                                              scores_.end(), v, n_);

  double score=0;
  for (unsigned int i=0; i< bestn.size(); ++i) {
    if (da) {
      bestn[i].second->evaluate(v, da);
    }
    score+= bestn[i].first;
  }
  return score;
}

ParticlesTemp MinimumCLASSNAMEScore
::get_input_particles(Particle* p) const {
  ParticlesTemp ret;
  for (unsigned int i=0; i< scores_.size(); ++i) {
    ParticlesTemp c= scores_[i]->get_input_particles(p);
    ret.insert(ret.end(), c.begin(), c.end());
  }
  return ret;
}

ContainersTemp MinimumCLASSNAMEScore
::get_input_containers(Particle* p) const {
  ContainersTemp ret;
  for (unsigned int i=0; i< scores_.size(); ++i) {
    ContainersTemp c= scores_[i]->get_input_containers(p);
    ret.insert(ret.end(), c.begin(), c.end());
  }
  return ret;
}


Restraints MinimumCLASSNAMEScore
::create_current_decomposition(ARGUMENTTYPE vt) const {
  Restraints ret;
  MinimumCLASSNAMEScoreMS bestn
    = find_minimal_set_MinimumCLASSNAMEScore(scores_.begin(),
                                              scores_.end(), vt, n_);
  for (unsigned int i=0; i< bestn.size(); ++i) {
    ret.push_back(new core::CLASSNAMERestraint(bestn[i].second, vt));
  }
  return ret;
}


void MinimumCLASSNAMEScore::do_show(std::ostream &out) const
{
  out << "size scores " << scores_.size() << std::endl;
}

IMPCONTAINER_END_NAMESPACE
