/**
 *  \file IMP/container/ClassnamesOptimizerState.h
 *  \brief Use a ClassnameModifier applied to a PLURALVARIABLETYPE to
 *  maintain an invariant
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_CLASSNAMES_OPTIMIZER_STATE_H
#define IMPCONTAINER_CLASSNAMES_OPTIMIZER_STATE_H

#include <IMP/container/container_config.h>
#include <IMP/ClassnameContainer.h>
#include <IMP/ClassnameModifier.h>
#include <IMP/OptimizerState.h>
#include <IMP/optimizer_state_macros.h>

IMPKERNEL_BEGIN_NAMESPACE
// for swig
class ClassnameContainer;
class ClassnameModifier;
IMPKERNEL_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE
//! Apply a ClassnameFunction to a ClassnameContainer to maintain an invariant
/** \ingroup restraint
    \see ClassnameOptimizerState
 */
class IMPCONTAINEREXPORT ClassnamesOptimizerState : public OptimizerState
{
  IMP::OwnerPointer<ClassnameModifier> f_;
  IMP::OwnerPointer<ClassnameContainer> c_;
public:
  /** \param[in] c The Container to hold the elements to process
      \param[in] gf The ClassnameModifier to apply to all elements.
      \param[in] name The name to use for this Object
   */
  ClassnamesOptimizerState(ClassnameContainerAdaptor c, ClassnameModifier *gf,
                           std::string name="ClassnamesOptimizerState %1%");

  IMP_OPTIMIZER_STATE(ClassnamesOptimizerState);
};

IMP_OBJECTS(ClassnamesOptimizerState,ClassnamesOptimizerStates);


IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_CLASSNAMES_OPTIMIZER_STATE_H */
