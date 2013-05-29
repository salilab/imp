/**
 *  \file domino/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/Order.h>
#include <IMP/domino/Subset.h>

IMPDOMINO_BEGIN_NAMESPACE

namespace {
Ints get_order_it(const Subset &s, const ParticlesTemp &all_particles) {
  Ints ret(s.size(), -1);
  int cur = 0;
  for (unsigned int i = 0; i < all_particles.size(); ++i) {
    for (unsigned int j = 0; j < s.size(); ++j) {
      if (all_particles[i] == s[j]) {
        ret[j] = cur;
        ++cur;
      }
    }
  }
  return ret;
}
}
Order::Order(Subset outer, const ParticlesTemp &order)
    : P(get_order_it(outer, order)) {}

IMPDOMINO_END_NAMESPACE
