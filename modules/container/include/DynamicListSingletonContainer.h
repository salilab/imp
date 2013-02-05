/**
 *  \file IMP/container/DynamicListSingletonContainer.h
 *  \brief Store a list of ParticlesTemp
 *
 *  WARNING This file was generated from DynamicListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_DYNAMIC_LIST_SINGLETON_CONTAINER_H
#define IMPCONTAINER_DYNAMIC_LIST_SINGLETON_CONTAINER_H

#include <IMP/container/container_config.h>
#include <IMP/internal/InternalDynamicListSingletonContainer.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Store a list of ParticlesTemp
/** In contrast to ListSingletonContainer, this list is designed to act
    as the output of a ScoreState or another container. The key difference
    is that it uses the passed Container to define the list of all
    possible contents of the container.
 */
class IMPCONTAINEREXPORT DynamicListSingletonContainer:
#if defined(IMP_DOXYGEN) || defined(SWIG)
public SingletonContainer
#else
public IMP::kernel::internal::InternalDynamicListSingletonContainer
#endif
{
  typedef IMP::kernel::internal::InternalDynamicListSingletonContainer P;
public:
  DynamicListSingletonContainer(Container *m,
                                std::string name= "ListSingletonContainer %1%");

 /** @name Methods to control the contained objects

     This container stores a list of Singleton objects. To manipulate
     the list use these methods.
  */
  /**@{*/
  void add_particle(Particle* vt);
  void add_particles(const ParticlesTemp &c);
  void set_particles(ParticlesTemp c);
  void clear_particles();
  /**@}*/
#ifdef SWIG
  IMP_SINGLETON_CONTAINER(DynamicListSingletonContainer);
#endif
};

IMP_OBJECTS(DynamicListSingletonContainer,DynamicListSingletonContainers);

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_DYNAMIC_LIST_SINGLETON_CONTAINER_H */
