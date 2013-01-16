/**
 *  \file IMP/declare_TripletContainer.h
 *  \brief A container for Triplets.
 *
 *  WARNING This file was generated from declare_NAMEContainer.hpp
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_DECLARE_TRIPLET_CONTAINER_H
#define IMPKERNEL_DECLARE_TRIPLET_CONTAINER_H

#include <IMP/kernel_config.h>
#include "internal/IndexingIterator.h"
#include "declare_Particle.h"
#include "container_base.h"
#include "internal/container_helpers.h"
#include "DerivativeAccumulator.h"
#include "internal/OwnerPointer.h"
#include "ParticleTuple.h"
#include <IMP/base/ref_counted_macros.h>
#include <IMP/base/check_macros.h>
#include <IMP/base/Pointer.h>
#include <IMP/base/InputAdaptor.h>
#include <IMP/base/utility_macros.h>
#include <IMP/base/deprecation_macros.h>
#include <algorithm>


IMP_BEGIN_NAMESPACE
class TripletModifier;
class TripletScore;

//! A shared container for Triplets
/** Stores a searchable shared collection of Triplets.
    \headerfile TripletContainer.h "IMP/TripletContainer.h"
    \implementationwithoutexample{TripletContainer, IMP_TRIPLET_CONTAINER}
 */
class IMPEXPORT TripletContainer : public Container
{
protected:
  TripletContainer(Model *m,
                           std::string name="TripletContainer %1%");
public:
  typedef ParticleTriplet ContainedType;
  typedef ParticleTripletsTemp ContainedTypes;
  typedef ParticleIndexTriplets ContainedIndexTypes;
  typedef ParticleIndexTriplet ContainedIndexType;

  //! Just use apply() in the base class
  void apply_generic(const TripletModifier *m) const;

  //! Apply a SingletonModifier to the contents
  void apply(const TripletModifier *sm) const;

  /** Get all the indexes contained in the container.*/
  virtual ParticleIndexTriplets get_indexes() const=0;
  /** Get all the indexes that might possibly be contained in the
      container, useful with dynamic containers.*/
  virtual ParticleIndexTriplets get_range_indexes() const=0;

#ifndef IMP_DOXYGEN
  ParticleTripletsTemp get() const {
    return IMP::internal::get_particle(get_model(),
                                       get_indexes());
  }

  ParticleTriplet get(unsigned int i) const {
    return IMP::internal::get_particle(get_model(),
                                       get_indexes()[i]);
  }
  unsigned int get_number() const {return get_indexes().size();}
#ifndef SWIG
  bool get_provides_access() const;
  virtual const ParticleIndexTriplets& get_access() const {
    IMP_THROW("Object not implemented properly.", base::IndexException);
  }


  template <class Functor>
    Functor for_each(Functor f) {
    ParticleIndexTriplets vs=get_indexes();
    // use boost range instead
    return std::for_each(vs.begin(), vs.end(), f);
  }

#endif
#endif

  /** Use this for debugging only.
   */
  ParticleTripletsTemp get_particle_triplets() const;

#if defined(IMP_USE_DEPRECATED)
  /** \brief This function is very slow and you should think hard about using
      it.

      \deprecated This is slow and dependent on the order of elements in the
      tuple.

      Return whether the container has the given element.*/
  IMP_DEPRECATED_WARN
    bool get_contains_particle_triplet(ParticleTriplet v) const;

  /** \deprecated This can be very slow and is probably not useful
   */
  IMP_DEPRECATED_WARN unsigned int get_number_of_particle_triplets() const;

  /** \deprecated Use indexes instead and thing about using the
      IMP_CONTAINER_FOREACH() macro.*/
  IMP_DEPRECATED_WARN ParticleTriplet
    get_particle_triplet(unsigned int i) const;

#endif
protected:
  virtual void do_apply(const TripletModifier *sm) const=0;
  virtual bool do_get_provides_access() const {return false;}

  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(TripletContainer);
};


/** This class allows either a list or a container to be
    accepted as input.
*/
class IMPEXPORT TripletContainerAdaptor:
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
public base::Pointer<TripletContainer>
#else
public base::InputAdaptor
#endif
{
  typedef base::Pointer<TripletContainer> P;
 public:
  TripletContainerAdaptor(){}
  TripletContainerAdaptor(TripletContainer *c);
  template <class C>
  TripletContainerAdaptor(base::internal::PointerBase<C> c): P(c){}
  TripletContainerAdaptor(const ParticleTripletsTemp &t,
                          std::string name="TripletContainerAdaptor%1%");
};


IMP_END_NAMESPACE

#endif  /* IMPKERNEL_DECLARE_TRIPLET_CONTAINER_H */
