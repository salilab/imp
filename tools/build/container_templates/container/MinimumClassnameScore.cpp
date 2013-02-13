/**
 *  \file MinimumClassnameScore.cpp  \brief Define ClassnameScore
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/container/MinimumClassnameScore.h>
#include <IMP/core/ClassnameRestraint.h>
#include "IMP/algebra/internal/MinimalSet.h"

IMPCONTAINER_BEGIN_NAMESPACE

MinimumClassnameScore::MinimumClassnameScore(const ClassnameScoresTemp &scores,
                                           unsigned int n,
                                           std::string name):
  ClassnameScore(name),
  scores_(scores.begin(), scores.end()),
  n_(n)
{
}


namespace {
  typedef algebra::internal::MinimalSet<double,
          ClassnameScore*, std::less<double> >
  MinimumClassnameScoreMS;
  template <class It>
  MinimumClassnameScoreMS
  find_minimal_set_MinimumClassnameScore(It b, It e,
                                         Model *m,
                                         PASSINDEXTYPE v,
                                          unsigned int n) {
    IMP_LOG(TERSE, "Finding Minimum " << n << " of "
            << std::distance(b,e) << std::endl);
    MinimumClassnameScoreMS bestn(n);
    for (It it= b; it != e; ++it) {
      double score= (*it)->evaluate_index(m, v, nullptr);
      bestn.insert(score, *it);
    }
    return bestn;
  }
}

double MinimumClassnameScore
::evaluate_index(Model *m, PASSINDEXTYPE v,
                 DerivativeAccumulator *da) const {
  MinimumClassnameScoreMS bestn
    = find_minimal_set_MinimumClassnameScore(scores_.begin(),
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

ModelObjectsTemp MinimumClassnameScore
::do_get_inputs(Model *m,
             const ParticleIndexes &pis) const {
  ModelObjectsTemp ret;
  for (unsigned int i=0; i< scores_.size(); ++i) {
    ret+= scores_[i]->get_inputs(m, pis);
  }
  return ret;
}

Restraints MinimumClassnameScore
::do_create_current_decomposition(Model *m,
                               PASSINDEXTYPE vt) const {
  Restraints ret;
  MinimumClassnameScoreMS bestn
    = find_minimal_set_MinimumClassnameScore(scores_.begin(),
                                             scores_.end(), m, vt, n_);
  for (unsigned int i=0; i< bestn.size(); ++i) {
    ret.push_back(IMP::kernel::internal::create_tuple_restraint(bestn[i].second,
                                                        m, vt));
    ret.back()->set_last_score(bestn[i].first);
  }
  return ret;
}


void MinimumClassnameScore::do_show(std::ostream &out) const
{
  out << "size scores " << scores_.size() << std::endl;
}

IMPCONTAINER_END_NAMESPACE
