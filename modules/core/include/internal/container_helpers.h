/**
 *  \file internal/particle_helpers.h
 *  \brief A container for Singletons.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_CONTAINER_HELPERS_H
#define IMPCORE_INTERNAL_CONTAINER_HELPERS_H

#include <IMP/core/core_config.h>
#include "../Typed.h"
#include <algorithm>
#include <IMP/particle_index.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

inline int get_ordered_type_hash(const ParticleTypes &rets) {
  int max = ParticleType::get_number_unique();
  int pow = 1;
  int ret = 0;
  for (unsigned int i = 0; i < rets.size(); ++i) {
    ret += pow * rets[i].get_index();
    pow *= max;
  }
  return ret;
}

inline int get_ordered_type_hash(Model *m, ParticleIndex pi) {
  Typed td(m, pi);
  return td.get_type().get_index();
}

//! n_types - number of unique particle types, if 0
//! then refetch number of possible particle types
//! (somewhat expensive operation for some reason)
template <unsigned int D>
inline int get_ordered_type_hash
(Model *m, const Array<D, ParticleIndex> &pi,
 int n_types=0)
{
  if(n_types==0){
    n_types = ParticleType::get_number_unique();
  }
  int pow = 1;
  int ret = 0;
  for (unsigned int i = 0; i < pi.size(); ++i) {
    Typed td(m, pi[i]);
    ret += pow * td.get_type().get_index();
    pow *= n_types;
  }
  return ret;
}

// faster version when done in batch (probably due to faster memore access)
inline int get_ordered_type_hash(ParticleIndex pi,
                                 int const* cached_particle_type_ids_table,
                                 int cached_n_particle_types // just for compatibility with D-dimensional template)
                                 ) {
  IMP_UNUSED(cached_n_particle_types);
  int type_id= cached_particle_type_ids_table[pi.get_index()];
  return ParticleType(type_id).get_index();
}

// faster version when done in batch (probably due to faster memore access)
template <unsigned int D>
inline int get_ordered_type_hash
(const Array<D, ParticleIndex> &pi,
 int const* cached_particle_type_ids_table, // table from model with particle type index per each particle index
 int cached_n_particle_types // total number of existing types, to make sure hash is unique
 )
{
  int pow = 1;
  int ret = 0;
  for (unsigned int i = 0; i < pi.size(); ++i) {
    int type_id= cached_particle_type_ids_table[pi[i].get_index()];
    ret += pow * ParticleType(type_id).get_index();
    pow *= cached_n_particle_types;
  }
  return ret;
}


inline int get_all_same(Model *, ParticleIndex) { return true; }

template <unsigned int D>
inline int get_all_same(Model *,
                        const Array<D, ParticleIndex> &pi) {
  for (unsigned int i = 1; i < D; ++i) {
    if (pi[i - 1] != pi[i]) return false;
  }
  return true;
}

inline int get_type_hash(Model *m, ParticleIndex pi) {
  return get_ordered_type_hash(m, pi);
}

template <unsigned int D>
inline int get_type_hash(Model *m,
                         const Array<D, ParticleIndex> &pi) {
  int max = ParticleType::get_number_unique();
  Ints rets(D);
  for (unsigned int i = 0; i < D; ++i) {
    Typed td(m, pi[i]);
    rets[i] = td.get_type().get_index();
  }
  std::sort(rets.begin(), rets.end());
  int pow = 1;
  int ret = 0;
  for (unsigned int i = 0; i < D; ++i) {
    ret += pow * rets[i];
    pow *= max;
  }
  return ret;
}

IMPCORE_END_INTERNAL_NAMESPACE

#endif /* IMPCORE_INTERNAL_CONTAINER_HELPERS_H */
