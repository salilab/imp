/**
 *  \file IMP/container/ListCLASSNAMEContainer.h
 *  \brief Store a list of PLURALVARIABLETYPE
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_LIST_HEADERNAME_CONTAINER_H
#define IMPCONTAINER_LIST_HEADERNAME_CONTAINER_H

#include <IMP/container/container_config.h>
#include <IMP/internal/InternalListCLASSNAMEContainer.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Store a list of PLURALVARIABLETYPE
/** \note The order can change when particles are inserted
    as the list is maintained in sorted order.
 */
class IMPCONTAINEREXPORT ListCLASSNAMEContainer:
#if defined(IMP_DOXYGEN) || defined(SWIG)
public CLASSNAMEContainer
#else
public IMP::internal::InternalListCLASSNAMEContainer
#endif
{
  typedef IMP::internal::InternalListCLASSNAMEContainer P;
public:
  ListCLASSNAMEContainer(Model *m,
                         const PLURALINDEXTYPE &contents,
                         std::string name= "ListCLASSNAMEContainer%1%");

  ListCLASSNAMEContainer(const PLURALVARIABLETYPE &ps,
                         std::string name= "ListCLASSNAMEContainer%1%");


  void set_FUNCTIONNAMEs(const PLURALVARIABLETYPE &ps);
  void set_FUNCTIONNAMEs(const PLURALINDEXTYPE &contents);
#ifndef IMP_DOXYGEN
  ListCLASSNAMEContainer(Model *m,
                         std::string name= "ListCLASSNAMEContainer %1%");
  ListCLASSNAMEContainer(Model *m, const char *name);

  void add_FUNCTIONNAME(ARGUMENTTYPE vt);
  void add_FUNCTIONNAMEs(const PLURALVARIABLETYPE &c);
  void clear_FUNCTIONNAMEs();
#endif
#ifdef SWIG
  IMP_HEADERNAME_CONTAINER(ListCLASSNAMEContainer);
#endif
};

IMP_OBJECTS(ListCLASSNAMEContainer,ListCLASSNAMEContainers);

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_LIST_HEADERNAME_CONTAINER_H */
