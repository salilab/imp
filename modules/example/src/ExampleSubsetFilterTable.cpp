/**
 *  \file ExampleSubsetFilterTable.cpp
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/example/ExampleSubsetFilterTable.h>
#include <iterator>

IMPEXAMPLE_BEGIN_NAMESPACE

namespace {
class ExampleSubsetFilter : public domino::SubsetFilter {
  Ints indices_;
  int max_;

 public:
  ExampleSubsetFilter(const Ints& indices, unsigned int max)
      : domino::SubsetFilter("ExampleSubsetFilter%1%"),
        indices_(indices),
        max_(max) {}
  virtual bool get_is_ok(const IMP::domino::Assignment&
                         assignment) const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(ExampleSubsetFilter);
};

bool ExampleSubsetFilter::get_is_ok(const domino::Assignment& a) const {
  for (unsigned int i = 0; i < indices_.size(); ++i) {
    if (indices_[i] == -1) continue;  // for the non-all case
    for (unsigned int j = 0; j < i; ++j) {
      if (indices_[j] == -1) continue;  // for the non-all case
      if (std::abs(a[indices_[i - 1]] - a[indices_[i]]) > max_) return false;
    }
  }
  return true;
}
}

ExampleSubsetFilterTable::ExampleSubsetFilterTable(unsigned int max_diff,
                                                   const ParticlesTemp& ps)
    : domino::SubsetFilterTable("ExampleSubsetFilterTable%1%"),
      max_diff_(max_diff),
      ps_(ps.begin(), ps.end()) {}

Ints ExampleSubsetFilterTable::get_indexes(
    const domino::Subset& s, const domino::Subsets& prior_subsets) const {
  // this method is only called from setup code, so it doesn't
  // have to be too fast
  Ints ret(ps_.size(), -1);
  for (unsigned int i = 0; i < s.size(); ++i) {
    for (unsigned int j = 0; j < ps_.size(); ++j) {
      if (s[i] == ps_[j]) {
        ret[j] = i;
        break;
      }
    }
  }
  for (unsigned int i = 0; i < ret.size(); ++i) {
    if (ret[i] == -1) return Ints();
  }
  // check if we have seen them already).
  for (unsigned int i = 0; i < prior_subsets.size(); ++i) {
    unsigned int count = 0;
    for (unsigned j = 0; j < prior_subsets[i].size(); ++j) {
      for (unsigned int k = 0; k < ps_.size(); ++k) {
        if (prior_subsets[i][j] == ps_[k]) {
          ++count;
          break;
        }
      }
    }
    if (count == ps_.size()) {
      return Ints();
    }
  }
  return ret;
}

double ExampleSubsetFilterTable::get_strength(
    const domino::Subset& cur_subset,
    const domino::Subsets& prior_subsets) const {
  IMP_OBJECT_LOG;
  if (get_indexes(cur_subset, prior_subsets).size() != ps_.size()) {
    return 0;
  } else {
    // pick some number
    return .5;
  }
}

domino::SubsetFilter* ExampleSubsetFilterTable::get_subset_filter(
    const domino::Subset& cur_subset,
    const domino::Subsets& prior_subsets) const {
  IMP_OBJECT_LOG;
  Ints its = get_indexes(cur_subset, prior_subsets);
  if (its.size() != ps_.size()) {
    // either the subset doesn't contain the needed particles or the prior does
    return nullptr;
  } else {
    IMP_NEW(ExampleSubsetFilter, ret, (its, max_diff_));
    // remember to release
    return ret.release();
  }
}

void ExampleSubsetFilterTable::do_show(std::ostream&) const {}

IMPEXAMPLE_END_NAMESPACE
