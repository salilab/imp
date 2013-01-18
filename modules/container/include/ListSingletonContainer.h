/**
 *  \file IMP/container/ListSingletonContainer.h
 *  \brief Store a list of ParticlesTemp
 *
 *  WARNING This file was generated from ListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_LIST_SINGLETON_CONTAINER_H
#define IMPCONTAINER_LIST_SINGLETON_CONTAINER_H

#include <IMP/container/container_config.h>
#include <IMP/internal/InternalListSingletonContainer.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Store a list of ParticlesTemp
/** \note The order can change when particles are inserted
    as the list is maintained in sorted order.
 */
class IMPCONTAINEREXPORT ListSingletonContainer:
#if defined(IMP_DOXYGEN) || defined(SWIG)
public SingletonContainer
#else
public IMP::internal::InternalListSingletonContainer
#endif
{
  typedef IMP::internal::InternalListSingletonContainer P;
public:
  ListSingletonContainer(const ParticleIndexes &contents,
                         std::string name= "ListSingletonContainer%1%");

  ListSingletonContainer(const ParticlesTemp &ps,
                         std::string name= "ListSingletonContainer%1%");

  void set_particles(const ParticlesTemp &ps);
  void set_particles(const ParticleIndexes &contents);
#ifndef IMP_DOXYGEN
  ListSingletonContainer(const ParticlesTemp &ps);

  //! construct and pass an initial set of Singletons
  ListSingletonContainer(const ParticlesTemp &ps,
                         std::string name);

  ListSingletonContainer(Model *m,
                         std::string name= "ListSingletonContainer %1%");
  ListSingletonContainer(Model *m, const char *name);

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
#endif
#ifdef SWIG
  IMP_SINGLETON_CONTAINER(ListSingletonContainer);
#endif
};

IMP_OBJECTS(ListSingletonContainer,ListSingletonContainers);

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_LIST_SINGLETON_CONTAINER_H */
