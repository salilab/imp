/**
 *  \file IMP/container/DynamicListQuadContainer.h
 *  \brief Store a list of ParticleQuadsTemp
 *
 *  WARNING This file was generated from DynamicListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_DYNAMIC_LIST_QUAD_CONTAINER_H
#define IMPCONTAINER_DYNAMIC_LIST_QUAD_CONTAINER_H

#include <IMP/container/container_config.h>
#include <IMP/internal/InternalDynamicListQuadContainer.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Store a list of ParticleQuadsTemp
/** In contrast to ListQuadContainer, this list is designed to act
    as the output of a ScoreState or another container. The key difference
    is that it uses the passed Container to define the list of all
    possible contents of the container.
 */
class IMPCONTAINEREXPORT DynamicListQuadContainer:
#if defined(IMP_DOXYGEN) || defined(SWIG)
public QuadContainer
#else
public IMP::kernel::internal::InternalDynamicListQuadContainer
#endif
{
  typedef IMP::kernel::internal::InternalDynamicListQuadContainer P;
public:
  DynamicListQuadContainer(Container *m,
                                std::string name= "ListQuadContainer %1%");

 /** @name Methods to control the contained objects

     This container stores a list of Quad objects. To manipulate
     the list use these methods.
  */
  /**@{*/
  void add_particle_quad(const ParticleQuad& vt);
  void add_particle_quads(const ParticleQuadsTemp &c);
  void set_particle_quads(ParticleQuadsTemp c);
  void clear_particle_quads();
  /**@}*/
#ifdef SWIG
  IMP_QUAD_CONTAINER(DynamicListQuadContainer);
#endif
};

IMP_OBJECTS(DynamicListQuadContainer,DynamicListQuadContainers);

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_DYNAMIC_LIST_QUAD_CONTAINER_H */
