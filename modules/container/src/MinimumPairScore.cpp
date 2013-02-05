/**
 *  \file MinimumPairScore.cpp  \brief Define PairScore
 *
 *  WARNING This file was generated from MinimumNAMEScore.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/container/MinimumPairScore.h>
#include <IMP/core/PairRestraint.h>
#include "IMP/algebra/internal/MinimalSet.h"

IMPCONTAINER_BEGIN_NAMESPACE

MinimumPairScore::MinimumPairScore(const PairScoresTemp &scores,
                                           unsigned int n,
                                           std::string name):
  PairScore(name),
  scores_(scores.begin(), scores.end()),
  n_(n)
{
}


namespace {
  typedef algebra::internal::MinimalSet<double,
          PairScore*, std::less<double> >
  MinimumPairScoreMS;
  template <class It>
  MinimumPairScoreMS
  find_minimal_set_MinimumPairScore(It b, It e,
                                         Model *m,
                                         const ParticleIndexPair& v,
                                          unsigned int n) {
    IMP_LOG(TERSE, "Finding Minimum " << n << " of "
            << std::distance(b,e) << std::endl);
    MinimumPairScoreMS bestn(n);
    for (It it= b; it != e; ++it) {
      double score= (*it)->evaluate_index(m, v, nullptr);
      bestn.insert(score, *it);
    }
    return bestn;
  }
}

double MinimumPairScore
::evaluate_index(Model *m, const ParticleIndexPair& v,
                 DerivativeAccumulator *da) const {
  MinimumPairScoreMS bestn
    = find_minimal_set_MinimumPairScore(scores_.begin(),
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

ModelObjectsTemp MinimumPairScore
::do_get_inputs(Model *m,
             const ParticleIndexes &pis) const {
  ModelObjectsTemp ret;
  for (unsigned int i=0; i< scores_.size(); ++i) {
    ret+= scores_[i]->get_inputs(m, pis);
  }
  return ret;
}

Restraints MinimumPairScore
::do_create_current_decomposition(Model *m,
                               const ParticleIndexPair& vt) const {
  Restraints ret;
  MinimumPairScoreMS bestn
    = find_minimal_set_MinimumPairScore(scores_.begin(),
                                             scores_.end(), m, vt, n_);
  for (unsigned int i=0; i< bestn.size(); ++i) {
    ret.push_back(IMP::kernel::internal::create_tuple_restraint(bestn[i].second,
                                                        m, vt));
    ret.back()->set_last_score(bestn[i].first);
  }
  return ret;
}


void MinimumPairScore::do_show(std::ostream &out) const
{
  out << "size scores " << scores_.size() << std::endl;
}

IMPCONTAINER_END_NAMESPACE
