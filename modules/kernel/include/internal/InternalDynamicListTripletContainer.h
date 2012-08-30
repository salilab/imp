/**
 *  \file ListTripletContainer.h
 *  \brief Store a list of ParticleTripletsTemp
 *
 *  WARNING This file was generated from InternalDynamicListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_DYNAMIC_LIST_TRIPLET_CONTAINER_H
#define IMPKERNEL_INTERNAL_INTERNAL_DYNAMIC_LIST_TRIPLET_CONTAINER_H

#include "../kernel_config.h"
#include "container_helpers.h"
#include "ListLikeTripletContainer.h"
#include <IMP/base/Pointer.h>

IMP_BEGIN_INTERNAL_NAMESPACE


class IMPEXPORT InternalDynamicListTripletContainer:
  public ListLikeTripletContainer
{
  typedef ListLikeTripletContainer P;
  // use this to define the set of all possible particles when it is dynamic
  base::Pointer<Container> scope_;
  bool check_list(const ParticleIndexes& cp) const;
 public:
  InternalDynamicListTripletContainer(Container *m, std::string name);
  InternalDynamicListTripletContainer(Container *m, const char *name);
  void add_particle_triplet(const ParticleTriplet& vt) {
    IMP_USAGE_CHECK(IMP::internal::is_valid(vt),
                    "Passed Triplet cannot be nullptr (or None)");

    add_to_list(IMP::internal::get_index(vt));
    IMP_USAGE_CHECK(check_list(IMP::internal::flatten
                               (IMP::internal::get_index(vt))),
                    "Invalid entries added to list " << vt);
  }
  void add_particle_triplet(const ParticleIndexTriplet& vt) {
    add_to_list(vt);
    IMP_USAGE_CHECK(check_list(IMP::internal::flatten(vt)),
                    "Invalid entries added to list " << vt);
  }
  void add_particle_triplets(const ParticleTripletsTemp &c) {
    if (c.empty()) return;
    ParticleIndexTriplets cp= IMP::internal::get_index(c);
    add_to_list(cp);
    IMP_USAGE_CHECK(check_list(IMP::internal::flatten
                               (cp)),
                    "Invalid entries added to list " << cp);
  }
  void remove_particle_triplets(const ParticleTripletsTemp &c);
  void set_particle_triplets(ParticleTripletsTemp c) {
    ParticleIndexTriplets cp= IMP::internal::get_index(c);
    update_list(cp);
    IMP_USAGE_CHECK(check_list(IMP::internal::flatten
                               (cp)),
                    "Invalid entries added to list " << c);
  }
  void set_particle_triplets(ParticleIndexTriplets cp) {
    update_list(cp);
    IMP_USAGE_CHECK(check_list(IMP::internal::flatten(cp)),
                    "Invalid entries added to list " << cp);
  }
  void clear_particle_triplets() {
    ParticleIndexTriplets t;
    update_list(t);
  }
  IMP_LISTLIKE_TRIPLET_CONTAINER(InternalDynamicListTripletContainer);
};

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_DYNAMIC_LIST_TRIPLET_CONTAINER_H */
