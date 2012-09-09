/**
 *  \file ListPairContainer.h    \brief Store a list of ParticlePairsTemp
 *
 *  WARNING This file was generated from InternalListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_LIST_PAIR_CONTAINER_H
#define IMPKERNEL_INTERNAL_INTERNAL_LIST_PAIR_CONTAINER_H

#include "../kernel_config.h"
#include "container_helpers.h"
#include "ListLikePairContainer.h"
#include <IMP/base/Pointer.h>

IMP_BEGIN_INTERNAL_NAMESPACE


class IMPEXPORT InternalListPairContainer:
  public ListLikePairContainer
{
  typedef ListLikePairContainer P;
 public:
  InternalListPairContainer(Model *m, std::string name);
  InternalListPairContainer(Model *m, const char *name);
  void add_particle_pair(const ParticlePair& vt) {
    get_model()->clear_caches();
    IMP_USAGE_CHECK(IMP::internal::is_valid(vt),
                    "Passed Pair cannot be nullptr (or None)");

    add_to_list(IMP::internal::get_index(vt));
  }
  void add_particle_pair(const ParticleIndexPair& vt) {
    get_model()->clear_caches();
    add_to_list(vt);
  }
  void add_particle_pairs(const ParticlePairsTemp &c) {
    if (c.empty()) return;
    get_model()->clear_caches();
    ParticleIndexPairs cp= IMP::internal::get_index(c);
    add_to_list(cp);
  }
  void remove_particle_pairs(const ParticlePairsTemp &c);
  void set_particle_pairs(ParticlePairsTemp c) {
    get_model()->clear_caches();
    ParticleIndexPairs cp= IMP::internal::get_index(c);
    update_list(cp);
  }
  void set_particle_pairs(ParticleIndexPairs cp) {
    get_model()->clear_caches();
    update_list(cp);
  }
  void clear_particle_pairs() {
    get_model()->clear_caches();
    ParticleIndexPairs t;
    update_list(t);
  }
  IMP_LISTLIKE_PAIR_CONTAINER(InternalListPairContainer);
};

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_LIST_PAIR_CONTAINER_H */
