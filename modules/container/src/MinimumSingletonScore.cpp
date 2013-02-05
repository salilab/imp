/**
 *  \file MinimumSingletonScore.cpp  \brief Define SingletonScore
 *
 *  WARNING This file was generated from MinimumNAMEScore.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/container/MinimumSingletonScore.h>
#include <IMP/core/SingletonRestraint.h>
#include "IMP/algebra/internal/MinimalSet.h"

IMPCONTAINER_BEGIN_NAMESPACE

MinimumSingletonScore::MinimumSingletonScore(const SingletonScoresTemp &scores,
                                           unsigned int n,
                                           std::string name):
  SingletonScore(name),
  scores_(scores.begin(), scores.end()),
  n_(n)
{
}


namespace {
  typedef algebra::internal::MinimalSet<double,
          SingletonScore*, std::less<double> >
  MinimumSingletonScoreMS;
  template <class It>
  MinimumSingletonScoreMS
  find_minimal_set_MinimumSingletonScore(It b, It e,
                                         Model *m,
                                         ParticleIndex v,
                                          unsigned int n) {
    IMP_LOG(TERSE, "Finding Minimum " << n << " of "
            << std::distance(b,e) << std::endl);
    MinimumSingletonScoreMS bestn(n);
    for (It it= b; it != e; ++it) {
      double score= (*it)->evaluate_index(m, v, nullptr);
      bestn.insert(score, *it);
    }
    return bestn;
  }
}

double MinimumSingletonScore
::evaluate_index(Model *m, ParticleIndex v,
                 DerivativeAccumulator *da) const {
  MinimumSingletonScoreMS bestn
    = find_minimal_set_MinimumSingletonScore(scores_.begin(),
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

ModelObjectsTemp MinimumSingletonScore
::do_get_inputs(Model *m,
             const ParticleIndexes &pis) const {
  ModelObjectsTemp ret;
  for (unsigned int i=0; i< scores_.size(); ++i) {
    ret+= scores_[i]->get_inputs(m, pis);
  }
  return ret;
}

Restraints MinimumSingletonScore
::do_create_current_decomposition(Model *m,
                               ParticleIndex vt) const {
  Restraints ret;
  MinimumSingletonScoreMS bestn
    = find_minimal_set_MinimumSingletonScore(scores_.begin(),
                                             scores_.end(), m, vt, n_);
  for (unsigned int i=0; i< bestn.size(); ++i) {
    ret.push_back(IMP::kernel::internal::create_tuple_restraint(bestn[i].second,
                                                        m, vt));
    ret.back()->set_last_score(bestn[i].first);
  }
  return ret;
}


void MinimumSingletonScore::do_show(std::ostream &out) const
{
  out << "size scores " << scores_.size() << std::endl;
}

IMPCONTAINER_END_NAMESPACE
