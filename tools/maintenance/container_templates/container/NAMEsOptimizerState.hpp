/**
 *  \file IMP/container/CLASSNAMEsOptimizerState.h
 *  \brief Use a CLASSNAMEModifier applied to a PLURALVARIABLETYPE to
 *  maintain an invariant
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_HEADERNAMES_OPTIMIZER_STATE_H
#define IMPCONTAINER_HEADERNAMES_OPTIMIZER_STATE_H

#include <IMP/container/container_config.h>
#include <IMP/CLASSNAMEContainer.h>
#include <IMP/CLASSNAMEModifier.h>
#include <IMP/OptimizerState.h>
#include <IMP/optimizer_state_macros.h>

IMPKERNEL_BEGIN_NAMESPACE
// for swig
class CLASSNAMEContainer;
class CLASSNAMEModifier;
IMPKERNEL_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE
//! Apply a CLASSNAMEFunction to a CLASSNAMEContainer to maintain an invariant
/** \ingroup restraint
    \see CLASSNAMEOptimizerState
 */
class IMPCONTAINEREXPORT CLASSNAMEsOptimizerState : public OptimizerState
{
  IMP::OwnerPointer<CLASSNAMEModifier> f_;
  IMP::OwnerPointer<CLASSNAMEContainer> c_;
public:
  /** \param[in] c The Container to hold the elements to process
      \param[in] gf The CLASSNAMEModifier to apply to all elements.
      \param[in] name The name to use for this Object
   */
  CLASSNAMEsOptimizerState(CLASSNAMEContainerAdaptor c, CLASSNAMEModifier *gf,
                           std::string name="CLASSNAMEsOptimizerState %1%");

  IMP_OPTIMIZER_STATE(CLASSNAMEsOptimizerState);
};

IMP_OBJECTS(CLASSNAMEsOptimizerState,CLASSNAMEsOptimizerStates);


IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_HEADERNAMES_OPTIMIZER_STATE_H */
