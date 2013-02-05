/**
 *  \file IMP/container/DynamicListCLASSNAMEContainer.h
 *  \brief Store a list of PLURALVARIABLETYPE
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_DYNAMIC_LIST_HEADERNAME_CONTAINER_H
#define IMPCONTAINER_DYNAMIC_LIST_HEADERNAME_CONTAINER_H

#include <IMP/container/container_config.h>
#include <IMP/internal/InternalDynamicListCLASSNAMEContainer.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Store a list of PLURALVARIABLETYPE
/** In contrast to ListCLASSNAMEContainer, this list is designed to act
    as the output of a ScoreState or another container. The key difference
    is that it uses the passed Container to define the list of all
    possible contents of the container.
 */
class IMPCONTAINEREXPORT DynamicListCLASSNAMEContainer:
#if defined(IMP_DOXYGEN) || defined(SWIG)
public CLASSNAMEContainer
#else
public IMP::kernel::internal::InternalDynamicListCLASSNAMEContainer
#endif
{
  typedef IMP::kernel::internal::InternalDynamicListCLASSNAMEContainer P;
public:
  DynamicListCLASSNAMEContainer(Container *m,
                                std::string name= "ListCLASSNAMEContainer %1%");

 /** @name Methods to control the contained objects

     This container stores a list of CLASSNAME objects. To manipulate
     the list use these methods.
  */
  /**@{*/
  void add_FUNCTIONNAME(ARGUMENTTYPE vt);
  void add_FUNCTIONNAMEs(const PLURALVARIABLETYPE &c);
  void set_FUNCTIONNAMEs(PLURALVARIABLETYPE c);
  void clear_FUNCTIONNAMEs();
  /**@}*/
#ifdef SWIG
  IMP_HEADERNAME_CONTAINER(DynamicListCLASSNAMEContainer);
#endif
};

IMP_OBJECTS(DynamicListCLASSNAMEContainer,DynamicListCLASSNAMEContainers);

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_DYNAMIC_LIST_HEADERNAME_CONTAINER_H */
