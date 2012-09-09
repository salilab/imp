/**
 *  \file ListSingletonContainer.h    \brief Store a list of ParticlesTemp
 *
 *  WARNING This file was generated from InternalListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_LIST_SINGLETON_CONTAINER_H
#define IMPKERNEL_INTERNAL_INTERNAL_LIST_SINGLETON_CONTAINER_H

#include "../kernel_config.h"
#include "container_helpers.h"
#include "ListLikeSingletonContainer.h"
#include <IMP/base/Pointer.h>

IMP_BEGIN_INTERNAL_NAMESPACE


class IMPEXPORT InternalListSingletonContainer:
  public ListLikeSingletonContainer
{
  typedef ListLikeSingletonContainer P;
 public:
  InternalListSingletonContainer(Model *m, std::string name);
  InternalListSingletonContainer(Model *m, const char *name);
  void add_particle(Particle* vt) {
    get_model()->clear_caches();
    IMP_USAGE_CHECK(IMP::internal::is_valid(vt),
                    "Passed Singleton cannot be nullptr (or None)");

    add_to_list(IMP::internal::get_index(vt));
  }
  void add_particle(ParticleIndex vt) {
    get_model()->clear_caches();
    add_to_list(vt);
  }
  void add_particles(const ParticlesTemp &c) {
    if (c.empty()) return;
    get_model()->clear_caches();
    ParticleIndexes cp= IMP::internal::get_index(c);
    add_to_list(cp);
  }
  void remove_particles(const ParticlesTemp &c);
  void set_particles(ParticlesTemp c) {
    get_model()->clear_caches();
    ParticleIndexes cp= IMP::internal::get_index(c);
    update_list(cp);
  }
  void set_particles(ParticleIndexes cp) {
    get_model()->clear_caches();
    update_list(cp);
  }
  void clear_particles() {
    get_model()->clear_caches();
    ParticleIndexes t;
    update_list(t);
  }
  IMP_LISTLIKE_SINGLETON_CONTAINER(InternalListSingletonContainer);
};

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_LIST_SINGLETON_CONTAINER_H */
