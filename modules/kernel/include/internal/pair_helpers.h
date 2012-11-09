/**
 *  \file internal/particle_pair_helpers.h
 *  \brief A container for Pairs.
 *
 *  WARNING This file was generated from name_helpers.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_PAIR_HELPERS_H
#define IMPKERNEL_INTERNAL_PAIR_HELPERS_H

#include "../kernel_config.h"
#include "../PairContainer.h"
#include "../PairModifier.h"
#include "../PairDerivativeModifier.h"
#include "../PairScore.h"
#include "container_helpers.h"
#include <algorithm>

IMP_BEGIN_NAMESPACE
class Model;
IMP_END_NAMESPACE

IMP_BEGIN_INTERNAL_NAMESPACE

template <class S>
inline void call_apply_index(Model *m, const S *s,
                      const ParticleIndexPair& a) {
  s->S::apply_index(m, a);
}
inline void call_apply(Model *m, const PairModifier *s,
                const ParticleIndexPair& a) {
  s->apply_index(m, a);
}
template <class S>
inline void call_apply_index(Model *m, const S *s,
                      const ParticleIndexPair& a,
                      DerivativeAccumulator *&da) {
  s->S::apply_index(m, a, da);
}

IMP_END_INTERNAL_NAMESPACE


#endif  /* IMPKERNEL_INTERNAL_PAIR_HELPERS_H */
