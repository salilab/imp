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
#include <IMP/internal/InternalListClassnameContainer.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Store a list of PLURALVARIABLETYPE
/** \note The order can change when particles are inserted
    as the list is maintained in sorted order.
 */
class IMPCONTAINEREXPORT ListClassnameContainer:
#if defined(IMP_DOXYGEN) || defined(SWIG)
public ClassnameContainer
#else
public IMP::kernel::internal::InternalListClassnameContainer
#endif
{
  typedef IMP::kernel::internal::InternalListClassnameContainer P;
public:
  ListClassnameContainer(Model *m,
                         const PLURALINDEXTYPE &contents,
                         std::string name= "ListClassnameContainer%1%");

  ListClassnameContainer(const PLURALVARIABLETYPE &ps,
                         std::string name= "ListClassnameContainer%1%");


  void set_FUNCTIONNAMEs(const PLURALVARIABLETYPE &ps);
  void set_FUNCTIONNAMEs(const PLURALINDEXTYPE &contents);
#ifndef IMP_DOXYGEN
  ListClassnameContainer(Model *m,
                         std::string name= "ListClassnameContainer %1%");
  ListClassnameContainer(Model *m, const char *name);

  void add_FUNCTIONNAME(ARGUMENTTYPE vt);
  void add_FUNCTIONNAMEs(const PLURALVARIABLETYPE &c);
  void clear_FUNCTIONNAMEs();
#endif
#ifdef SWIG
  IMP_CLASSNAME_CONTAINER(ListClassnameContainer);
#endif
};

IMP_OBJECTS(ListClassnameContainer,ListClassnameContainers);

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_LIST_CLASSNAME_CONTAINER_H */
