/**
 *  \file IMP/ClassnameContainer.h
 *  \brief A container for Classnames.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_CLASSNAME_CONTAINER_H
#define IMPKERNEL_CLASSNAME_CONTAINER_H

#include <IMP/kernel_config.h>
#include "internal/IndexingIterator.h"
#include "Particle.h"
#include "container_base.h"
#include "internal/container_helpers.h"
#include "DerivativeAccumulator.h"
#include "base_types.h"
#include <IMP/ref_counted_macros.h>
#include <IMP/check_macros.h>
#include <IMP/Pointer.h>
#include <IMP/InputAdaptor.h>
#include <IMP/utility_macros.h>
#include <IMP/deprecation_macros.h>
#include <algorithm>

IMPKERNEL_BEGIN_NAMESPACE
class ClassnameModifier;
class ClassnameScore;

//! A shared container for Classnames
/** Stores a shared collection of Classnames.
 */
class IMPKERNELEXPORT ClassnameContainer : public Container {
 public:
  typedef VARIABLETYPE ContainedType;
  typedef PLURALVARIABLETYPE ContainedTypes;
  typedef PLURALINDEXTYPE ContainedIndexTypes;
  typedef INDEXTYPE ContainedIndexType;
  typedef ClassnameModifier Modifier;
  typedef PASSINDEXTYPE PassContainedIndexType;

  //! Just use apply() in the base class
  void apply_generic(const ClassnameModifier *m) const;

  //! Apply a SingletonModifier to the contents
  void apply(const ClassnameModifier *sm) const;

  /** Get all the indexes that might possibly be contained in the
      container, useful with dynamic containers. For example,
      with a container::ClosePairContainer, this is the list
      of all pairs taken from input list (those that are far apart
      in addition to those that are close).
  */
  virtual PLURALINDEXTYPE get_range_indexes() const = 0;

  const PLURALINDEXTYPE &get_contents() const {
    if (get_provides_access())
      return get_access();
    else {
      std::size_t nhash = get_contents_hash();
      if (contents_hash_ != nhash || !cache_initialized_) {
        contents_hash_ = nhash;
        cache_initialized_ = true;
        get_indexes_in_place(contents_cache_); // inheriting class could implement this faster than get_indexes()
      }
      return contents_cache_;
    }
  }

  /** Get all the indexes contained in the container.

    This should be protected but isn't for compatibility reasons.

    External callers should use get_contents().
  */
  virtual PLURALINDEXTYPE get_indexes() const = 0;


#ifndef IMP_DOXYGEN

  PLURALVARIABLETYPE get() const {
    return IMP::internal::get_particle(get_model(), get_indexes());
  }

  VARIABLETYPE get(unsigned int i) const {
    return IMP::internal::get_particle(get_model(), get_indexes()[i]);
  }

  //! Return size of current container content
  //! Note that this may be expensive since my evaluate
  //! refreshing of the container
  unsigned int get_number() const { return get_indexes().size(); }
#ifndef SWIG
  bool get_provides_access() const;
  virtual const PLURALINDEXTYPE &get_access() const {
    IMP_THROW("Object not implemented properly.", IndexException);
  }

  template <class Functor>
  Functor for_each(Functor f) {
    PLURALINDEXTYPE vs = get_indexes();
    // use boost range instead
    return std::for_each(vs.begin(), vs.end(), f);
  }

#endif
#endif

  /** \deprecated_at{2.1}
      Use get_contents() instead.
   */
  PLURALVARIABLETYPE get_FUNCTIONNAMEs() const;

  /** \deprecated_at{2.1}
      Use get_contents() instead and consider using the
      IMP_CONTAINER_FOREACH() macro.*/
  IMPKERNEL_DEPRECATED_METHOD_DECL(2.1)
  VARIABLETYPE get_FUNCTIONNAME(unsigned int i) const;

 protected:
  ClassnameContainer(Model *m,
                     std::string name = "ClassnameContainer %1%");

  virtual void do_apply(const ClassnameModifier *sm) const = 0;
  virtual bool do_get_provides_access() const { return false; }

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  //! Insert the contents of the container into output
  //! Could be reimplemented to save time in terms of e.g. memory
  //! allocation compared with calling get_indexes()
  virtual void get_indexes_in_place(PLURALINDEXTYPE& output) const{
    output= get_indexes();
  }
#endif

  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(ClassnameContainer);

 private:
  mutable std::size_t contents_hash_;
  mutable PLURALINDEXTYPE contents_cache_;
  mutable bool cache_initialized_;
};

/** This class allows either a list or a container to be
    accepted as input.
*/
class IMPKERNELEXPORT ClassnameContainerAdaptor :
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
    public Pointer<ClassnameContainer>
#else
    public InputAdaptor
#endif
    {
  typedef Pointer<ClassnameContainer> P;

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
  ClassnameContainerAdaptor(IMP::internal::PointerBase<C> c)
      : P(c) {}

  /**
     Adapts the non-empty list t to ClassnameContainer

     @param t a non-empty list of PLURALVARIABLETYPE
  */
  ClassnameContainerAdaptor(const PLURALVARIABLETYPE &t);

  /** Set the name of the resulting container if it is currently the
      default value. */
  void set_name_if_default(std::string name);
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_CLASSNAME_CONTAINER_H */
