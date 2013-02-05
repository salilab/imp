/**
 *  \file IMP/container/TripletsOptimizerState.h
 *  \brief Use a TripletModifier applied to a ParticleTripletsTemp to
 *  maintain an invariant
 *
 *  WARNING This file was generated from NAMEsOptimizerState.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_TRIPLETS_OPTIMIZER_STATE_H
#define IMPCONTAINER_TRIPLETS_OPTIMIZER_STATE_H

#include <IMP/container/container_config.h>
#include <IMP/TripletContainer.h>
#include <IMP/TripletModifier.h>
#include <IMP/OptimizerState.h>
#include <IMP/optimizer_state_macros.h>

IMPKERNEL_BEGIN_NAMESPACE
// for swig
class TripletContainer;
class TripletModifier;
IMPKERNEL_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE
//! Apply a TripletFunction to a TripletContainer to maintain an invariant
/** \ingroup restraint
    \see TripletOptimizerState
 */
class IMPCONTAINEREXPORT TripletsOptimizerState : public OptimizerState
{
  IMP::OwnerPointer<TripletModifier> f_;
  IMP::OwnerPointer<TripletContainer> c_;
public:
  /** \param[in] c The Container to hold the elements to process
      \param[in] gf The TripletModifier to apply to all elements.
      \param[in] name The name to use for this Object
   */
  TripletsOptimizerState(TripletContainerAdaptor c, TripletModifier *gf,
                           std::string name="TripletsOptimizerState %1%");

  IMP_OPTIMIZER_STATE(TripletsOptimizerState);
};

IMP_OBJECTS(TripletsOptimizerState,TripletsOptimizerStates);


IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_TRIPLETS_OPTIMIZER_STATE_H */
