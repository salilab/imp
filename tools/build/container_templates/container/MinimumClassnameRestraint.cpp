/**
 *  \file MinimumClassnameRestraint.cpp
 *  \brief The minimum score over a container.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/MinimumClassnameRestraint.h"
#include "IMP/algebra/internal/MinimalSet.h"
#include <IMP/internal/container_helpers.h>
#include <IMP/core/ClassnameRestraint.h>
#include <IMP/classname_macros.h>
#include <IMP/container_macros.h>
#include <IMP/internal/TupleRestraint.h>

IMPCONTAINER_BEGIN_NAMESPACE

MinimumClassnameRestraint
::MinimumClassnameRestraint(ClassnameScore *f,
                           ClassnameContainerAdaptor c,
                           unsigned int n,
                           std::string name):
  Restraint(c->get_model(), name),
  f_(f), c_(c), n_(n){
}

namespace {
  typedef algebra::internal::MinimalSet<double,
          INDEXTYPE, std::less<double> > ClassnameMinimumMS;
  template <class C, class F>
  ClassnameMinimumMS find_minimal_set_ClassnameMinimum(C* c, F *f,
                                                         unsigned int n) {
    IMP_LOG(VERBOSE, "Finding Minimum " << n << " of "
            << c->get_number() << std::endl);
    ClassnameMinimumMS bestn(n);
    IMP_CONTAINER_FOREACH_TEMPLATE(C, c, {
        double score= f->evaluate_index(c->get_model(),
                                        _1, nullptr);
        IMP_LOG(VERBOSE, "Found " << score << " for "
                << _1 << std::endl);
        bestn.insert(score, _1);
      });
    return bestn;
  }
}

double MinimumClassnameRestraint
::unprotected_evaluate(DerivativeAccumulator *da) const {
  IMP_OBJECT_LOG;
  ClassnameMinimumMS bestn
    = find_minimal_set_ClassnameMinimum(c_.get(),
                                         f_.get(), n_);

  double score=0;
  for (unsigned int i=0; i< bestn.size(); ++i) {
    if (da) {
      f_->evaluate_index(get_model(), bestn[i].second, da);
    }
    score+= bestn[i].first;
  }
  IMP_LOG(VERBOSE, "Total score is " << score << std::endl);
  return score;
}

double MinimumClassnameRestraint
::unprotected_evaluate_if_good(DerivativeAccumulator *da,
                               double max) const {
  IMP_OBJECT_LOG;
  ClassnameMinimumMS bestn
    = find_minimal_set_ClassnameMinimum(c_.get(),
                                         f_.get(), n_);

  double score=0;
  for (unsigned int i=0; i< bestn.size(); ++i) {
    if (da) {
      f_->evaluate_index(get_model(), bestn[i].second, da);
    }
    score+= bestn[i].first;
    if (score > max) break;
  }
  IMP_LOG(VERBOSE, "Total score is " << score << std::endl);
  return score;
}

Restraints MinimumClassnameRestraint
::do_create_current_decomposition() const {
  IMP_OBJECT_LOG;
  ClassnameMinimumMS bestn
    = find_minimal_set_ClassnameMinimum(c_.get(),
                                         f_.get(), n_);
  Restraints ret;
  for (unsigned int i=0; i< bestn.size(); ++i) {
    ret.push_back(IMP::kernel::internal::create_tuple_restraint(f_.get(),
                                                        get_model(),
                                                        bestn[i].second,
                                                        get_name()));
    ret.back()->set_last_score(bestn[i].first);
  }
  return ret;
}

ModelObjectsTemp MinimumClassnameRestraint::do_get_inputs() const
{
  ModelObjectsTemp ret;
  ret+=f_->get_inputs(get_model(),
                      c_->get_all_possible_indexes());
  ret.push_back(c_);
  return ret;
}



IMPCONTAINER_END_NAMESPACE
