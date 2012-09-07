/**
 *  \file ListQuadContainer.h    \brief Store a list of ParticleQuadsTemp
 *
 *  WARNING This file was generated from InternalListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_LIST_QUAD_CONTAINER_H
#define IMPKERNEL_INTERNAL_INTERNAL_LIST_QUAD_CONTAINER_H

#include "../kernel_config.h"
#include "container_helpers.h"
#include "ListLikeQuadContainer.h"
#include <IMP/base/Pointer.h>

IMP_BEGIN_INTERNAL_NAMESPACE


class IMPEXPORT InternalListQuadContainer:
  public ListLikeQuadContainer
{
  typedef ListLikeQuadContainer P;
 public:
  InternalListQuadContainer(Model *m, std::string name);
  InternalListQuadContainer(Model *m, const char *name);
  void add_particle_quad(const ParticleQuad& vt) {
    get_model()->reset_caches();
    IMP_USAGE_CHECK(IMP::internal::is_valid(vt),
                    "Passed Quad cannot be nullptr (or None)");

    add_to_list(IMP::internal::get_index(vt));
  }
  void add_particle_quad(const ParticleIndexQuad& vt) {
    get_model()->reset_caches();
    add_to_list(vt);
  }
  void add_particle_quads(const ParticleQuadsTemp &c) {
    if (c.empty()) return;
    get_model()->reset_caches();
    ParticleIndexQuads cp= IMP::internal::get_index(c);
    add_to_list(cp);
  }
  void remove_particle_quads(const ParticleQuadsTemp &c);
  void set_particle_quads(ParticleQuadsTemp c) {
    get_model()->reset_caches();
    ParticleIndexQuads cp= IMP::internal::get_index(c);
    update_list(cp);
  }
  void set_particle_quads(ParticleIndexQuads cp) {
    get_model()->reset_caches();
    update_list(cp);
  }
  void clear_particle_quads() {
    get_model()->reset_caches();
    ParticleIndexQuads t;
    update_list(t);
  }
  IMP_LISTLIKE_QUAD_CONTAINER(InternalListQuadContainer);
};

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_LIST_QUAD_CONTAINER_H */
