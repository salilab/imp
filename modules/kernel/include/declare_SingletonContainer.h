/**
 *  \file IMP/declare_SingletonContainer.h
 *  \brief A container for Singletons.
 *
 *  WARNING This file was generated from declare_NAMEContainer.hpp
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_DECLARE_SINGLETON_CONTAINER_H
#define IMPKERNEL_DECLARE_SINGLETON_CONTAINER_H

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
class SingletonModifier;
class SingletonScore;

//! A shared container for Singletons
/** Stores a searchable shared collection of Singletons.
    \headerfile SingletonContainer.h "IMP/SingletonContainer.h"
    \implementationwithoutexample{SingletonContainer, IMP_SINGLETON_CONTAINER}
 */
class IMPEXPORT SingletonContainer : public Container
{
protected:
  SingletonContainer(Model *m,
                           std::string name="SingletonContainer %1%");
public:
  typedef Particle* ContainedType;
  typedef ParticlesTemp ContainedTypes;
  typedef ParticleIndexes ContainedIndexTypes;
  typedef ParticleIndex ContainedIndexType;

  //! Just use apply() in the base class
  void apply_generic(const SingletonModifier *m) const;

  //! Apply a SingletonModifier to the contents
  void apply(const SingletonModifier *sm) const;

  /** Get all the indexes contained in the container.*/
  virtual ParticleIndexes get_indexes() const=0;
  /** Get all the indexes that might possibly be contained in the
      container, useful with dynamic containers.*/
  virtual ParticleIndexes get_range_indexes() const=0;

#ifndef IMP_DOXYGEN
  ParticlesTemp get() const {
    return IMP::internal::get_particle(get_model(),
                                       get_indexes());
  }

  Particle* get(unsigned int i) const {
    return IMP::internal::get_particle(get_model(),
                                       get_indexes()[i]);
  }
  unsigned int get_number() const {return get_indexes().size();}
#ifndef SWIG
  bool get_provides_access() const;
  virtual const ParticleIndexes& get_access() const {
    IMP_THROW("Object not implemented properly.", base::IndexException);
  }


  template <class Functor>
    Functor for_each(Functor f) {
    ParticleIndexes vs=get_indexes();
    // use boost range instead
    return std::for_each(vs.begin(), vs.end(), f);
  }

#endif
#endif

  /** Use this for debugging only.
   */
  ParticlesTemp get_particles() const;

#if defined(IMP_USE_DEPRECATED)
  /** \brief This function is very slow and you should think hard about using
      it.

      \deprecated This is slow and dependent on the order of elements in the
      tuple.

      Return whether the container has the given element.*/
  IMP_DEPRECATED_WARN
    bool get_contains_particle(Particle* v) const;

  /** \deprecated This can be very slow and is probably not useful
   */
  IMP_DEPRECATED_WARN unsigned int get_number_of_particles() const;

  /** \deprecated Use indexes instead and thing about using the
      IMP_CONTAINER_FOREACH() macro.*/
  IMP_DEPRECATED_WARN Particle*
    get_particle(unsigned int i) const;

#endif
protected:
  virtual void do_apply(const SingletonModifier *sm) const=0;
  virtual bool do_get_provides_access() const {return false;}

  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(SingletonContainer);
};


/** This class allows either a list or a container to be
    accepted as input.
*/
class IMPEXPORT SingletonContainerAdaptor:
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
public base::Pointer<SingletonContainer>
#else
public base::InputAdaptor
#endif
{
  typedef base::Pointer<SingletonContainer> P;
 public:
  SingletonContainerAdaptor(){}
  SingletonContainerAdaptor(SingletonContainer *c);
  template <class C>
  SingletonContainerAdaptor(base::internal::PointerBase<C> c): P(c){}
  SingletonContainerAdaptor(const ParticlesTemp &t,
                          std::string name="SingletonContainerAdaptor%1%");
};


IMP_END_NAMESPACE

#endif  /* IMPKERNEL_DECLARE_SINGLETON_CONTAINER_H */
