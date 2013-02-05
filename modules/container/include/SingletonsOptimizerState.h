/**
 *  \file IMP/container/SingletonsOptimizerState.h
 *  \brief Use a SingletonModifier applied to a ParticlesTemp to
 *  maintain an invariant
 *
 *  WARNING This file was generated from NAMEsOptimizerState.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_SINGLETONS_OPTIMIZER_STATE_H
#define IMPCONTAINER_SINGLETONS_OPTIMIZER_STATE_H

#include <IMP/container/container_config.h>
#include <IMP/SingletonContainer.h>
#include <IMP/SingletonModifier.h>
#include <IMP/OptimizerState.h>
#include <IMP/optimizer_state_macros.h>

IMPKERNEL_BEGIN_NAMESPACE
// for swig
class SingletonContainer;
class SingletonModifier;
IMPKERNEL_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE
//! Apply a SingletonFunction to a SingletonContainer to maintain an invariant
/** \ingroup restraint
    \see SingletonOptimizerState
 */
class IMPCONTAINEREXPORT SingletonsOptimizerState : public OptimizerState
{
  IMP::OwnerPointer<SingletonModifier> f_;
  IMP::OwnerPointer<SingletonContainer> c_;
public:
  /** \param[in] c The Container to hold the elements to process
      \param[in] gf The SingletonModifier to apply to all elements.
      \param[in] name The name to use for this Object
   */
  SingletonsOptimizerState(SingletonContainerAdaptor c, SingletonModifier *gf,
                           std::string name="SingletonsOptimizerState %1%");

  IMP_OPTIMIZER_STATE(SingletonsOptimizerState);
};

IMP_OBJECTS(SingletonsOptimizerState,SingletonsOptimizerStates);


IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_SINGLETONS_OPTIMIZER_STATE_H */
