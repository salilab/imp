/**
 *  \file IMP/container/TripletsConstraint.h
 *  \brief Use a TripletModifier applied to a ParticleTripletsTemp to
 *  maintain an invariant
 *
 *  WARNING This file was generated from NAMEsConstraint.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_TRIPLETS_CONSTRAINT_H
#define IMPCONTAINER_TRIPLETS_CONSTRAINT_H

#include <IMP/container/container_config.h>
#include <IMP/TripletContainer.h>
#include <IMP/TripletModifier.h>
#include <IMP/Constraint.h>
#include <IMP/score_state_macros.h>
#include <IMP/internal/ContainerConstraint.h>

IMP_BEGIN_NAMESPACE
// for swig
class TripletContainer;
class TripletModifier;
IMP_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE
//! Apply a TripletFunction to a TripletContainer to maintain an invariant
/** The score state is passed up to two TripletModifiers, one to
    apply before evaluation and the other after. The one after
    should take a DerivativeAccumulator as its last argument for
    TripletModifier::apply() and will only be called if
    the score was computed with derivatives.

    An example showing a how to use such a score state to maintain a cover
    of the atoms of a protein by a sphere per residue.
    \verbinclude cover_particles.py

    \see core::TripletConstraint
 */
class IMPCONTAINEREXPORT TripletsConstraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
 public Constraint
#else
 public IMP::internal::ContainerConstraint<TripletModifier,
                                           TripletDerivativeModifier,
                                           TripletContainer>
#endif
{
  typedef IMP::internal::ContainerConstraint<TripletModifier,
                                           TripletDerivativeModifier,
      TripletContainer> P;
public:
  /** \param[in] c The Container to hold the elements to process
      \param[in] before The TripletModifier to apply to all elements
      before evaluate.
      \param[in] after The TripletModifier to apply to all elements
      after evaluate.
      \param[in] name The object name
   */
  TripletsConstraint(TripletModifier *before,
                       TripletDerivativeModifier *after,
                       TripletContainerAdaptor c,
                       std::string name="TripletConstraint %1%"):
      P(before, after, c, name)
      {}
#if defined(IMP_DOXYGEN) || defined(SWIG)
protected:
  void do_update_attributes();
  void do_update_derivatives(DerivativeAccumulator *da);
  virtual ModelObjectsTemp do_get_inputs() const;
  virtual ModelObjectsTemp do_get_outputs() const;
  IMP_OBJECT_METHODS(TripletsConstraint);
#endif
};

IMP_OBJECTS(TripletsConstraint,TripletsConstraints);


IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_TRIPLETS_CONSTRAINT_H */
