/**
 *  \file IMP/container/DynamicListTripletContainer.h
 *  \brief Store a list of ParticleTripletsTemp
 *
 *  WARNING This file was generated from DynamicListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_DYNAMIC_LIST_TRIPLET_CONTAINER_H
#define IMPCONTAINER_DYNAMIC_LIST_TRIPLET_CONTAINER_H

#include <IMP/container/container_config.h>
#include <IMP/internal/InternalDynamicListTripletContainer.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Store a list of ParticleTripletsTemp
/** In contrast to ListTripletContainer, this list is designed to act
    as the output of a ScoreState or another container. The key difference
    is that it uses the passed Container to define the list of all
    possible contents of the container.
 */
class IMPCONTAINEREXPORT DynamicListTripletContainer:
#if defined(IMP_DOXYGEN) || defined(SWIG)
public TripletContainer
#else
public IMP::kernel::internal::InternalDynamicListTripletContainer
#endif
{
  typedef IMP::kernel::internal::InternalDynamicListTripletContainer P;
public:
  DynamicListTripletContainer(Container *m,
                                std::string name= "ListTripletContainer %1%");

 /** @name Methods to control the contained objects

     This container stores a list of Triplet objects. To manipulate
     the list use these methods.
  */
  /**@{*/
  void add_particle_triplet(const ParticleTriplet& vt);
  void add_particle_triplets(const ParticleTripletsTemp &c);
  void set_particle_triplets(ParticleTripletsTemp c);
  void clear_particle_triplets();
  /**@}*/
#ifdef SWIG
  IMP_TRIPLET_CONTAINER(DynamicListTripletContainer);
#endif
};

IMP_OBJECTS(DynamicListTripletContainer,DynamicListTripletContainers);

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_DYNAMIC_LIST_TRIPLET_CONTAINER_H */
