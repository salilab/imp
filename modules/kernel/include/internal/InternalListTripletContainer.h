/**
 *  \file ListTripletContainer.h    \brief Store a list of ParticleTripletsTemp
 *
 *  WARNING This file was generated from InternalListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_LIST_TRIPLET_CONTAINER_H
#define IMPKERNEL_INTERNAL_INTERNAL_LIST_TRIPLET_CONTAINER_H

#include "../kernel_config.h"
#include "container_helpers.h"
#include "ListLikeTripletContainer.h"
#include <IMP/base/Pointer.h>

IMP_BEGIN_INTERNAL_NAMESPACE


class IMPEXPORT InternalListTripletContainer:
  public ListLikeTripletContainer
{
  typedef ListLikeTripletContainer P;
 public:
  InternalListTripletContainer(Model *m, std::string name);
  InternalListTripletContainer(Model *m, const char *name);
  void add_particle_triplet(const ParticleTriplet& vt) {
    get_model()->clear_caches();
    IMP_USAGE_CHECK(IMP::internal::is_valid(vt),
                    "Passed Triplet cannot be nullptr (or None)");

    add_to_list(IMP::internal::get_index(vt));
  }
  void add_particle_triplet(const ParticleIndexTriplet& vt) {
    get_model()->clear_caches();
    add_to_list(vt);
  }
  void add_particle_triplets(const ParticleTripletsTemp &c) {
    if (c.empty()) return;
    get_model()->clear_caches();
    ParticleIndexTriplets cp= IMP::internal::get_index(c);
    add_to_list(cp);
  }
  void remove_particle_triplets(const ParticleTripletsTemp &c);
  void set_particle_triplets(ParticleTripletsTemp c) {
    get_model()->clear_caches();
    ParticleIndexTriplets cp= IMP::internal::get_index(c);
    update_list(cp);
  }
  void set_particle_triplets(ParticleIndexTriplets cp) {
    get_model()->clear_caches();
    update_list(cp);
  }
  void clear_particle_triplets() {
    get_model()->clear_caches();
    ParticleIndexTriplets t;
    update_list(t);
  }
  IMP_LISTLIKE_TRIPLET_CONTAINER(InternalListTripletContainer);
};

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_LIST_TRIPLET_CONTAINER_H */
