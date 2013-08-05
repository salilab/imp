/**
 *  \file IMP/kernel/declare_ClassnameContainer.h
 *  \brief A container for Classnames.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_DECLARE_CLASSNAME_CONTAINER_H
#define IMPKERNEL_DECLARE_CLASSNAME_CONTAINER_H

#include <IMP/kernel/kernel_config.h>
#include "internal/IndexingIterator.h"
#include "declare_Particle.h"
#include "container_base.h"
#include "internal/container_helpers.h"
#include "DerivativeAccumulator.h"
#include "ParticleTuple.h"
#include <IMP/base/ref_counted_macros.h>
#include <IMP/base/check_macros.h>
#include <IMP/base/Pointer.h>
#include <IMP/base/InputAdaptor.h>
#include <IMP/base/utility_macros.h>
#include <IMP/base/deprecation_macros.h>
#include <algorithm>

IMPKERNEL_BEGIN_NAMESPACE
class ClassnameModifier;
class ClassnameScore;

//! A shared container for Classnames
/** Stores a searchable shared collection of Classnames.
    \headerfile ClassnameContainer.h "IMP/kernel/ClassnameContainer.h"
    \implementationwithoutexample{ClassnameContainer, IMP_CLASSNAME_CONTAINER}
 */
class IMPKERNELEXPORT ClassnameContainer : public Container {
 protected:
  ClassnameContainer(Model *m, std::string name = "ClassnameContainer %1%");

 public:
  typedef VARIABLETYPE ContainedType;
  typedef PLURALVARIABLETYPE ContainedTypes;
  typedef PLURALINDEXTYPE ContainedIndexTypes;
  typedef INDEXTYPE ContainedIndexType;

  //! Just use apply() in the base class
  void apply_generic(const ClassnameModifier *m) const;

  //! Apply a SingletonModifier to the contents
  void apply(const ClassnameModifier *sm) const;

  /** Get all the indexes contained in the container.*/
  virtual PLURALINDEXTYPE get_indexes() const = 0;
  /** Get all the indexes that might possibly be contained in the
      container, useful with dynamic containers. For example,
      with a container::ClosePairContainer, this is the list
      of all pairs taken from input list (those that are far apart
      in addition to those that are close).
  */
  virtual PLURALINDEXTYPE get_range_indexes() const = 0;

#ifndef IMP_DOXYGEN
  PLURALVARIABLETYPE get() const {
    return IMP::kernel::internal::get_particle(get_model(), get_indexes());
  }

  VARIABLETYPE get(unsigned int i) const {
    return IMP::kernel::internal::get_particle(get_model(), get_indexes()[i]);
  }
  unsigned int get_number() const { return get_indexes().size(); }
#ifndef SWIG
  bool get_provides_access() const;
  virtual const PLURALINDEXTYPE &get_access() const {
    IMP_THROW("Object not implemented properly.", base::IndexException);
  }

  template <class Functor> Functor for_each(Functor f) {
    PLURALINDEXTYPE vs = get_indexes();
    // use boost range instead
    return std::for_each(vs.begin(), vs.end(), f);
  }

#endif
#endif

  /** Use this for debugging only.
   */
  PLURALVARIABLETYPE get_FUNCTIONNAMEs() const;

  /** \brief This function is very slow and you should think hard about using
      it.

      \deprecated_at{2.1} This is slow and dependent on the order of elements
      in the tuple.

      Return whether the container has the given element.*/
  IMPKERNEL_DEPRECATED_METHOD_DECL(2.1)
    bool get_contains_FUNCTIONNAME(VARIABLETYPE v) const;

  /** \deprecated_at{2.1} This can be very slow and is probably not useful
   */
  IMPKERNEL_DEPRECATED_METHOD_DECL(2.1)
    unsigned int get_number_of_FUNCTIONNAMEs() const;

  /** \deprecated_at{2.1}Use indexes instead and thing about using the
      IMP_CONTAINER_FOREACH() macro.*/
  IMPKERNEL_DEPRECATED_METHOD_DECL(2.1)
    VARIABLETYPE get_FUNCTIONNAME(unsigned int i) const;

 protected:
  virtual void do_apply(const ClassnameModifier *sm) const = 0;
  virtual bool do_get_provides_access() const { return false; }

  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(ClassnameContainer);
};

/** This class allows either a list or a container to be
    accepted as input.
*/
class IMPKERNELEXPORT ClassnameContainerAdaptor :
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
    public base::Pointer<ClassnameContainer>
#else
    public base::InputAdaptor
#endif
    {
  typedef base::Pointer<ClassnameContainer> P;

 public:
  ClassnameContainerAdaptor() {}

  /**
     Constructs the adaptor pointing to c (so if the contents of c are changed
     dynamically, so do the contents of the adaptor, and vice versa)
   */
  ClassnameContainerAdaptor(ClassnameContainer *c);

  /**
     Constructs the adaptor pointing to c (so if the contents of c are changed
     dynamically, so do the contents of the adaptor, and vice versa)
   */
  template <class C>
  ClassnameContainerAdaptor(base::internal::PointerBase<C> c)
      : P(c) {}

  /**
     Adapts the non-empty list t to ClassnameContainer

     @param t a non-empty list of PLURALVARIABLETYPE
     @param name name for the constructed ClassnameContainer
  */
  ClassnameContainerAdaptor(const PLURALVARIABLETYPE &t,
                            std::string name = "ClassnameContainerAdaptor%1%");
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_DECLARE_CLASSNAME_CONTAINER_H */
