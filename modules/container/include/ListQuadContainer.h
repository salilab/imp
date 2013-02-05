/**
 *  \file IMP/container/ListQuadContainer.h
 *  \brief Store a list of ParticleQuadsTemp
 *
 *  WARNING This file was generated from ListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_LIST_QUAD_CONTAINER_H
#define IMPCONTAINER_LIST_QUAD_CONTAINER_H

#include <IMP/container/container_config.h>
#include <IMP/internal/InternalListQuadContainer.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Store a list of ParticleQuadsTemp
/** \note The order can change when particles are inserted
    as the list is maintained in sorted order.
 */
class IMPCONTAINEREXPORT ListQuadContainer:
#if defined(IMP_DOXYGEN) || defined(SWIG)
public QuadContainer
#else
public IMP::kernel::internal::InternalListQuadContainer
#endif
{
  typedef IMP::kernel::internal::InternalListQuadContainer P;
public:
  ListQuadContainer(Model *m,
                         const ParticleIndexQuads &contents,
                         std::string name= "ListQuadContainer%1%");

  ListQuadContainer(const ParticleQuadsTemp &ps,
                         std::string name= "ListQuadContainer%1%");


  void set_particle_quads(const ParticleQuadsTemp &ps);
  void set_particle_quads(const ParticleIndexQuads &contents);
#ifndef IMP_DOXYGEN
  ListQuadContainer(Model *m,
                         std::string name= "ListQuadContainer %1%");
  ListQuadContainer(Model *m, const char *name);

  void add_particle_quad(const ParticleQuad& vt);
  void add_particle_quads(const ParticleQuadsTemp &c);
  void clear_particle_quads();
#endif
#ifdef SWIG
  IMP_QUAD_CONTAINER(ListQuadContainer);
#endif
};

IMP_OBJECTS(ListQuadContainer,ListQuadContainers);

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_LIST_QUAD_CONTAINER_H */
