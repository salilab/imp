/**
 *  \file IMP/container/QuadsOptimizerState.h
 *  \brief Use a QuadModifier applied to a ParticleQuadsTemp to
 *  maintain an invariant
 *
 *  WARNING This file was generated from NAMEsOptimizerState.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_QUADS_OPTIMIZER_STATE_H
#define IMPCONTAINER_QUADS_OPTIMIZER_STATE_H

#include <IMP/container/container_config.h>
#include <IMP/QuadContainer.h>
#include <IMP/QuadModifier.h>
#include <IMP/OptimizerState.h>
#include <IMP/optimizer_state_macros.h>

IMPKERNEL_BEGIN_NAMESPACE
// for swig
class QuadContainer;
class QuadModifier;
IMPKERNEL_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE
//! Apply a QuadFunction to a QuadContainer to maintain an invariant
/** \ingroup restraint
    \see QuadOptimizerState
 */
class IMPCONTAINEREXPORT QuadsOptimizerState : public OptimizerState
{
  IMP::OwnerPointer<QuadModifier> f_;
  IMP::OwnerPointer<QuadContainer> c_;
public:
  /** \param[in] c The Container to hold the elements to process
      \param[in] gf The QuadModifier to apply to all elements.
      \param[in] name The name to use for this Object
   */
  QuadsOptimizerState(QuadContainerAdaptor c, QuadModifier *gf,
                           std::string name="QuadsOptimizerState %1%");

  IMP_OPTIMIZER_STATE(QuadsOptimizerState);
};

IMP_OBJECTS(QuadsOptimizerState,QuadsOptimizerStates);


IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_QUADS_OPTIMIZER_STATE_H */
