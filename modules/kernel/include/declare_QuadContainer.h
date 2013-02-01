/**
 *  \file IMP/declare_QuadContainer.h
 *  \brief A container for Quads.
 *
 *  WARNING This file was generated from declare_NAMEContainer.hpp
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_DECLARE_QUAD_CONTAINER_H
#define IMPKERNEL_DECLARE_QUAD_CONTAINER_H

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
class QuadModifier;
class QuadScore;

//! A shared container for Quads
/** Stores a searchable shared collection of Quads.
    \headerfile QuadContainer.h "IMP/QuadContainer.h"
    \implementationwithoutexample{QuadContainer, IMP_QUAD_CONTAINER}
 */
class IMPEXPORT QuadContainer : public Container
{
protected:
  QuadContainer(Model *m,
                           std::string name="QuadContainer %1%");
public:
  typedef ParticleQuad ContainedType;
  typedef ParticleQuadsTemp ContainedTypes;
  typedef ParticleIndexQuads ContainedIndexTypes;
  typedef ParticleIndexQuad ContainedIndexType;

  //! Just use apply() in the base class
  void apply_generic(const QuadModifier *m) const;

  //! Apply a SingletonModifier to the contents
  void apply(const QuadModifier *sm) const;

  /** Get all the indexes contained in the container.*/
  virtual ParticleIndexQuads get_indexes() const=0;
  /** Get all the indexes that might possibly be contained in the
      container, useful with dynamic containers. For example,
      with a container::ClosePairContainer, this is the list
      of all pairs taken from input list (those that are far apart
      in addition to those that are close).
  */
  virtual ParticleIndexQuads get_range_indexes() const=0;

#ifndef IMP_DOXYGEN
  ParticleQuadsTemp get() const {
    return IMP::internal::get_particle(get_model(),
                                       get_indexes());
  }

  ParticleQuad get(unsigned int i) const {
    return IMP::internal::get_particle(get_model(),
                                       get_indexes()[i]);
  }
  unsigned int get_number() const {return get_indexes().size();}
#ifndef SWIG
  bool get_provides_access() const;
  virtual const ParticleIndexQuads& get_access() const {
    IMP_THROW("Object not implemented properly.", base::IndexException);
  }


  template <class Functor>
    Functor for_each(Functor f) {
    ParticleIndexQuads vs=get_indexes();
    // use boost range instead
    return std::for_each(vs.begin(), vs.end(), f);
  }

#endif
#endif

  /** Use this for debugging only.
   */
  ParticleQuadsTemp get_particle_quads() const;

#if defined(IMP_USE_DEPRECATED)
  /** \brief This function is very slow and you should think hard about using
      it.

      \deprecated This is slow and dependent on the order of elements in the
      tuple.

      Return whether the container has the given element.*/
  IMP_DEPRECATED_WARN
    bool get_contains_particle_quad(ParticleQuad v) const;

  /** \deprecated This can be very slow and is probably not useful
   */
  IMP_DEPRECATED_WARN unsigned int get_number_of_particle_quads() const;

  /** \deprecated Use indexes instead and thing about using the
      IMP_CONTAINER_FOREACH() macro.*/
  IMP_DEPRECATED_WARN ParticleQuad
    get_particle_quad(unsigned int i) const;

#endif
protected:
  virtual void do_apply(const QuadModifier *sm) const=0;
  virtual bool do_get_provides_access() const {return false;}

  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(QuadContainer);
};


/** This class allows either a list or a container to be
    accepted as input.
*/
class IMPEXPORT QuadContainerAdaptor:
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
public base::Pointer<QuadContainer>
#else
public base::InputAdaptor
#endif
{
  typedef base::Pointer<QuadContainer> P;
 public:
  QuadContainerAdaptor(){}
  QuadContainerAdaptor(QuadContainer *c);
  template <class C>
  QuadContainerAdaptor(base::internal::PointerBase<C> c): P(c){}
  QuadContainerAdaptor(const ParticleQuadsTemp &t,
                          std::string name="QuadContainerAdaptor%1%");
};


IMP_END_NAMESPACE

#endif  /* IMPKERNEL_DECLARE_QUAD_CONTAINER_H */
