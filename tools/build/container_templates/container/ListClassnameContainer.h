/**
 *  \file IMP/container/ListClassnameContainer.h
 *  \brief Store a list of PLURALINDEXTYPE.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_LIST_CLASSNAME_CONTAINER_H
#define IMPCONTAINER_LIST_CLASSNAME_CONTAINER_H

#include <IMP/container/container_config.h>
#include <IMP/object_macros.h>
#include <IMP/internal/StaticListContainer.h>
#include <IMP/ClassnameContainer.h>
#include <IMP/ClassnameModifier.h>
#include <IMP/base_types.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Store a list of PLURALINDEXTYPE
/** \note The order may change when particles are inserted
    as the list is maintained in sorted order.

    \note Changing the contents of the container is a fairly heavy
    weight operation as it resets part of the dependency graph. Use
    a DynamicListClassnameContainer if you want to change the contents
    frequently (eg via a ScoreState).
 */
class IMPCONTAINEREXPORT ListClassnameContainer :
#if defined(IMP_DOXYGEN) || defined(SWIG)
    public ClassnameContainer
#else
    public IMP::internal::StaticListContainer<ClassnameContainer>
#endif
    {
  typedef IMP::internal::StaticListContainer<ClassnameContainer> P;

 public:
  ListClassnameContainer(Model *m, const PLURALINDEXTYPE &contents,
                         std::string name = "ListClassnameContainer%1%");

#ifndef IMP_DOXYGEN
  ListClassnameContainer(Model *m,
                         std::string name = "ListClassnameContainer %1%");
  ListClassnameContainer(Model *m, const char *name);

#endif
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

#ifdef SWIG
  PLURALINDEXTYPE get_indexes() const;
  PLURALINDEXTYPE get_range_indexes() const;
  ModelObjectsTemp do_get_inputs() const;
  void do_apply(const ClassnameModifier *sm) const;
  ParticleIndexes get_all_possible_indexes() const;

 private:
  std::size_t do_get_contents_hash() const;
#endif
  IMP_OBJECT_METHODS(ListClassnameContainer);
};

IMP_OBJECTS(ListClassnameContainer, ListClassnameContainers);

IMPCONTAINER_END_NAMESPACE

#endif /* IMPCONTAINER_LIST_CLASSNAME_CONTAINER_H */
