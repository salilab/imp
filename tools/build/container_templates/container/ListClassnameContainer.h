/**
 *  \file IMP/container/ListClassnameContainer.h
 *  \brief Store a list of PLURALVARIABLETYPE
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_LIST_CLASSNAME_CONTAINER_H
#define IMPCONTAINER_LIST_CLASSNAME_CONTAINER_H

#include <IMP/container/container_config.h>
#include <IMP/base/object_macros.h>
#include <IMP/kernel/internal/StaticListContainer.h>
#include <IMP/kernel/ClassnameContainer.h>
#include <IMP/kernel/ClassnameModifier.h>


IMPCONTAINER_BEGIN_NAMESPACE

//! Store a PLURALINDEXTYPE
/** \note The order may change when particles are inserted
    as the list is maintained in sorted order.

    \note Changing the contents of the container is a fairly heavy
    weight operation as it resets part of the dependency graph. Use
    a DynamicListClassnameContainer if you want to change the contents
    frequently (eg via a kernel::ScoreState).
 */
class IMPCONTAINEREXPORT ListClassnameContainer :
#if defined(IMP_DOXYGEN) || defined(SWIG)
    public ClassnameContainer
#else
    public IMP::kernel::internal::StaticListContainer<
        kernel::ClassnameContainer>
#endif
    {
  typedef IMP::kernel::internal::StaticListContainer<kernel::ClassnameContainer>
      P;

 public:
  ListClassnameContainer(kernel::Model *m, const PLURALINDEXTYPE &contents,
                         std::string name = "ListClassnameContainer%1%");

  ListClassnameContainer(const PLURALVARIABLETYPE &ps,
                         std::string name = "ListClassnameContainer%1%");

  void set_FUNCTIONNAMEs(const PLURALVARIABLETYPE &ps);
  void set_FUNCTIONNAMEs(const PLURALINDEXTYPE &contents);
#ifndef IMP_DOXYGEN
  ListClassnameContainer(kernel::Model *m,
                         std::string name = "ListClassnameContainer %1%");
  ListClassnameContainer(kernel::Model *m, const char *name);

  void add_FUNCTIONNAME(ARGUMENTTYPE vt);
  void add_FUNCTIONNAMEs(const PLURALVARIABLETYPE &c);
  void clear_FUNCTIONNAMEs();
#endif
#ifdef SWIG
  PLURALINDEXTYPE get_indexes() const;
  PLURALINDEXTYPE get_range_indexes() const;
  kernel::ModelObjectsTemp do_get_inputs() const;
  void do_apply(const ClassnameModifier *sm) const;
  kernel::ParticleIndexes get_all_possible_indexes() const;

 private:
  std::size_t do_get_contents_hash() const;
#endif
  IMP_OBJECT_METHODS(ListClassnameContainer);
};

IMP_OBJECTS(ListClassnameContainer, ListClassnameContainers);

IMPCONTAINER_END_NAMESPACE

#endif /* IMPCONTAINER_LIST_CLASSNAME_CONTAINER_H */
