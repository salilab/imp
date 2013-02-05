/**
 *  \file IMP/container/DynamicListPairContainer.h
 *  \brief Store a list of ParticlePairsTemp
 *
 *  WARNING This file was generated from DynamicListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_DYNAMIC_LIST_PAIR_CONTAINER_H
#define IMPCONTAINER_DYNAMIC_LIST_PAIR_CONTAINER_H

#include <IMP/container/container_config.h>
#include <IMP/internal/InternalDynamicListPairContainer.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Store a list of ParticlePairsTemp
/** In contrast to ListPairContainer, this list is designed to act
    as the output of a ScoreState or another container. The key difference
    is that it uses the passed Container to define the list of all
    possible contents of the container.
 */
class IMPCONTAINEREXPORT DynamicListPairContainer:
#if defined(IMP_DOXYGEN) || defined(SWIG)
public PairContainer
#else
public IMP::kernel::internal::InternalDynamicListPairContainer
#endif
{
  typedef IMP::kernel::internal::InternalDynamicListPairContainer P;
public:
  DynamicListPairContainer(Container *m,
                                std::string name= "ListPairContainer %1%");

 /** @name Methods to control the contained objects

     This container stores a list of Pair objects. To manipulate
     the list use these methods.
  */
  /**@{*/
  void add_particle_pair(const ParticlePair& vt);
  void add_particle_pairs(const ParticlePairsTemp &c);
  void set_particle_pairs(ParticlePairsTemp c);
  void clear_particle_pairs();
  /**@}*/
#ifdef SWIG
  IMP_PAIR_CONTAINER(DynamicListPairContainer);
#endif
};

IMP_OBJECTS(DynamicListPairContainer,DynamicListPairContainers);

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_DYNAMIC_LIST_PAIR_CONTAINER_H */
