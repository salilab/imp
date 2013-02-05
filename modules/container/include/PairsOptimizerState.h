/**
 *  \file IMP/container/PairsOptimizerState.h
 *  \brief Use a PairModifier applied to a ParticlePairsTemp to
 *  maintain an invariant
 *
 *  WARNING This file was generated from NAMEsOptimizerState.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_PAIRS_OPTIMIZER_STATE_H
#define IMPCONTAINER_PAIRS_OPTIMIZER_STATE_H

#include <IMP/container/container_config.h>
#include <IMP/PairContainer.h>
#include <IMP/PairModifier.h>
#include <IMP/OptimizerState.h>
#include <IMP/optimizer_state_macros.h>

IMPKERNEL_BEGIN_NAMESPACE
// for swig
class PairContainer;
class PairModifier;
IMPKERNEL_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE
//! Apply a PairFunction to a PairContainer to maintain an invariant
/** \ingroup restraint
    \see PairOptimizerState
 */
class IMPCONTAINEREXPORT PairsOptimizerState : public OptimizerState
{
  IMP::OwnerPointer<PairModifier> f_;
  IMP::OwnerPointer<PairContainer> c_;
public:
  /** \param[in] c The Container to hold the elements to process
      \param[in] gf The PairModifier to apply to all elements.
      \param[in] name The name to use for this Object
   */
  PairsOptimizerState(PairContainerAdaptor c, PairModifier *gf,
                           std::string name="PairsOptimizerState %1%");

  IMP_OPTIMIZER_STATE(PairsOptimizerState);
};

IMP_OBJECTS(PairsOptimizerState,PairsOptimizerStates);


IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_PAIRS_OPTIMIZER_STATE_H */
