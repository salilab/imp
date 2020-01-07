/**
 *  \file IMP/container/DynamicListClassnameContainer.h
 *  \brief Store a list of PLURALVARIABLETYPE
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_DYNAMIC_LIST_CLASSNAME_CONTAINER_H
#define IMPCONTAINER_DYNAMIC_LIST_CLASSNAME_CONTAINER_H

#include <IMP/container/container_config.h>
#include <IMP/object_macros.h>
#include <IMP/internal/DynamicListContainer.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Store a PLURALINDEXTYPE
/** In contrast to ListClassnameContainer, this list is designed to act
    as the output of a ScoreState or another container. The key difference
    is that it uses the passed Container to define the list of all
    possible contents of the container.
 */
class IMPCONTAINEREXPORT DynamicListClassnameContainer :
#if defined(IMP_DOXYGEN) || defined(SWIG)
    public ClassnameContainer
#else
    public IMP::internal::DynamicListContainer<ClassnameContainer>
#endif
    {
  typedef IMP::internal::DynamicListContainer<ClassnameContainer> P;

 public:
  /** Constructs the dynamic list

      @param scope a container with a list of all possible particles that
                   are allowed to be dynamically added or removed in this
                   container
      @param name  the name of this container
  */
  DynamicListClassnameContainer(Container *scope, std::string name =
                                                  "ListClassnameContainer %1%");

  /** @name Methods to control the contained objects

      This container stores a list of Classname objects. To manipulate
      the list use these methods.
   */
  /**@{*/
#if defined(SWIG) || defined(IMP_DOXYGEN)
  //! Add a single PASSINDEXTYPE to the container.
  void add(PASSINDEXTYPE vt);

  //! Add PLURALINDEXTYPE to the container.
  void add(const PLURALINDEXTYPE &c);

  //! Set the contents of the container to the given PLURALINDEXTYPE.
  void set(PLURALINDEXTYPE cp);

  //! Clear the contents of the container.
  void clear();
#endif

/**@}*/
#ifdef SWIG
  PLURALINDEXTYPE get_indexes() const;
  PLURALINDEXTYPE get_range_indexes() const;
  ModelObjectsTemp do_get_inputs() const;
  void do_apply(const ClassnameModifier *sm) const;
  ParticleIndexes get_all_possible_indexes() const;
#endif
  IMP_OBJECT_METHODS(DynamicListClassnameContainer);
};

IMP_OBJECTS(DynamicListClassnameContainer, DynamicListClassnameContainers);

IMPCONTAINER_END_NAMESPACE

#endif /* IMPCONTAINER_DYNAMIC_LIST_CLASSNAME_CONTAINER_H */
