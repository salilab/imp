/**
 *  \file MinimumCLASSNAMEScore.cpp  \brief Define CLASSNAMEScore
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
  find_minimal_set_MinimumCLASSNAMEScore(It b, It e,
                                         Model *m,
                                         PASSINDEXTYPE v,
                                          unsigned int n) {
    IMP_LOG(TERSE, "Finding Minimum " << n << " of "
            << std::distance(b,e) << std::endl);
    MinimumCLASSNAMEScoreMS bestn(n);
    for (It it= b; it != e; ++it) {
      double score= (*it)->evaluate_index(m, v, nullptr);
      bestn.insert(score, *it);
    }
    return bestn;
  }
}

double MinimumCLASSNAMEScore
::evaluate_index(Model *m, PASSINDEXTYPE v,
                 DerivativeAccumulator *da) const {
  MinimumCLASSNAMEScoreMS bestn
    = find_minimal_set_MinimumCLASSNAMEScore(scores_.begin(),
                                             scores_.end(), m, v, n_);

  double score=0;
  for (unsigned int i=0; i< bestn.size(); ++i) {
    if (da) {
      bestn[i].second->evaluate_index(m, v, da);
    }
    score+= bestn[i].first;
  }
  return score;
}

ModelObjectsTemp MinimumCLASSNAMEScore
::do_get_inputs(Model *m,
             const ParticleIndexes &pis) const {
  ModelObjectsTemp ret;
  for (unsigned int i=0; i< scores_.size(); ++i) {
    ret+= scores_[i]->get_inputs(m, pis);
  }
  return ret;
}

Restraints MinimumCLASSNAMEScore
::do_create_current_decomposition(Model *m,
                               PASSINDEXTYPE vt) const {
  Restraints ret;
  MinimumCLASSNAMEScoreMS bestn
    = find_minimal_set_MinimumCLASSNAMEScore(scores_.begin(),
                                             scores_.end(), m, vt, n_);
  for (unsigned int i=0; i< bestn.size(); ++i) {
    ret.push_back(IMP::kernel::internal::create_tuple_restraint(bestn[i].second,
                                                        m, vt));
    ret.back()->set_last_score(bestn[i].first);
  }
  return ret;
}


void MinimumCLASSNAMEScore::do_show(std::ostream &out) const
{
  out << "size scores " << scores_.size() << std::endl;
}

IMPCONTAINER_END_NAMESPACE
