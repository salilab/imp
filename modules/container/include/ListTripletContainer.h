/**
 *  \file IMP/container/ListTripletContainer.h
 *  \brief Store a list of ParticleTripletsTemp
 *
 *  WARNING This file was generated from ListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_LIST_TRIPLET_CONTAINER_H
#define IMPCONTAINER_LIST_TRIPLET_CONTAINER_H

#include <IMP/container/container_config.h>
#include <IMP/internal/InternalListTripletContainer.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Store a list of ParticleTripletsTemp
/** \note The order can change when particles are inserted
    as the list is maintained in sorted order.
 */
class IMPCONTAINEREXPORT ListTripletContainer:
#if defined(IMP_DOXYGEN) || defined(SWIG)
public TripletContainer
#else
public IMP::internal::InternalListTripletContainer
#endif
{
  typedef IMP::internal::InternalListTripletContainer P;
public:
  ListTripletContainer(Model *m,
                         const ParticleIndexTriplets &contents,
                         std::string name= "ListTripletContainer%1%");

  ListTripletContainer(const ParticleTripletsTemp &ps,
                         std::string name= "ListTripletContainer%1%");


  void set_particle_triplets(const ParticleTripletsTemp &ps);
  void set_particle_triplets(const ParticleIndexTriplets &contents);
#ifndef IMP_DOXYGEN
  ListTripletContainer(Model *m,
                         std::string name= "ListTripletContainer %1%");
  ListTripletContainer(Model *m, const char *name);

  void add_particle_triplet(const ParticleTriplet& vt);
  void add_particle_triplets(const ParticleTripletsTemp &c);
  void clear_particle_triplets();
#endif
#ifdef SWIG
  IMP_TRIPLET_CONTAINER(ListTripletContainer);
#endif
};

IMP_OBJECTS(ListTripletContainer,ListTripletContainers);

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_LIST_TRIPLET_CONTAINER_H */
