/**
 *  \file SubsetMover.cpp
 *  \brief  A mover that applies a random subset of movers
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core/SubsetMover.h>
#include <IMP/random_utils.h>
#include <algorithm>

IMPCORE_BEGIN_NAMESPACE

SubsetMover::SubsetMover(const MonteCarloMoversTemp& mvs, unsigned int n)
    : MonteCarloMover(IMP::internal::get_model(mvs), "SubsetMover%1%"),
      movers_(mvs), n_(n) {
        IMP_USAGE_CHECK(n_ > 0,
                        "size of sample must be greater than 0.");
        IMP_USAGE_CHECK(n_ <= movers_.size(),
                        "size of sample cannot be greater than number of movers.");
        subset_inds_ = std::vector<unsigned int>(n, 0);
      }

MonteCarloMoverResult SubsetMover::do_propose() {
  unsigned int i;
  Vector<double> us;
  std::vector<std::pair<double, unsigned int>> ordered(movers_.size());

  get_random_numbers_uniform<double>(us, movers_.size());

  for (i = 0; i < movers_.size(); ++i) {
    ordered[i] = std::pair<double, unsigned int>(us[i], i);
  }
  std::sort(ordered.begin(), ordered.end());

  for (i = 0; i < n_; ++i) {
    subset_inds_[i] = ordered[i].second;
  }

  ParticleIndexes pis;
  for (i = 0; i < subset_inds_.size(); ++i) {
    pis += movers_[subset_inds_[i]]->propose().get_moved_particles();
  }

  return MonteCarloMoverResult(pis, 1.0);
}

void SubsetMover::do_reject() {
  for (int i = n_ - 1; i >= 0; --i) {
    movers_[subset_inds_[i]]->reject();
  }
}

void SubsetMover::do_accept() {
  for (unsigned int i = 0; i < n_; ++i) {
    movers_[subset_inds_[i]]->accept();
  }
}

ModelObjectsTemp SubsetMover::do_get_inputs() const {
  ModelObjectsTemp ret;
  for (unsigned int i = 0; i < movers_.size(); ++i) {
    ret += movers_[i]->get_inputs();
  }
  return ret;
}

IMPCORE_END_NAMESPACE
