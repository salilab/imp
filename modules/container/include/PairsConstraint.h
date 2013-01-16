/**
 *  \file IMP/container/PairsConstraint.h
 *  \brief Use a PairModifier applied to a ParticlePairsTemp to
 *  maintain an invariant
 *
 *  WARNING This file was generated from NAMEsConstraint.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_PAIRS_CONSTRAINT_H
#define IMPCONTAINER_PAIRS_CONSTRAINT_H

#include <IMP/container/container_config.h>
#include <IMP/PairContainer.h>
#include <IMP/PairModifier.h>
#include <IMP/Constraint.h>
#include <IMP/score_state_macros.h>
#include <IMP/internal/ContainerConstraint.h>

IMP_BEGIN_NAMESPACE
// for swig
class PairContainer;
class PairModifier;
IMP_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE
//! Apply a PairFunction to a PairContainer to maintain an invariant
/** The score state is passed up to two PairModifiers, one to
    apply before evaluation and the other after. The one after
    should take a DerivativeAccumulator as its last argument for
    PairModifier::apply() and will only be called if
    the score was computed with derivatives.

    An example showing a how to use such a score state to maintain a cover
    of the atoms of a protein by a sphere per residue.
    \verbinclude cover_particles.py

    \see core::PairConstraint
 */
class IMPCONTAINEREXPORT PairsConstraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
 public Constraint
#else
 public IMP::internal::ContainerConstraint<PairModifier,
                                           PairDerivativeModifier,
                                           PairContainer>
#endif
{
  typedef IMP::internal::ContainerConstraint<PairModifier,
                                           PairDerivativeModifier,
      PairContainer> P;
public:
  /** \param[in] c The Container to hold the elements to process
      \param[in] before The PairModifier to apply to all elements
      before evaluate.
      \param[in] after The PairModifier to apply to all elements
      after evaluate.
      \param[in] name The object name
   */
  PairsConstraint(PairModifier *before,
                       PairDerivativeModifier *after,
                       PairContainerAdaptor c,
                       std::string name="PairConstraint %1%"):
      P(before, after, c, name)
      {}
#if defined(IMP_DOXYGEN) || defined(SWIG)
protected:
  void do_update_attributes();
  void do_update_derivatives(DerivativeAccumulator *da);
  virtual ModelObjectsTemp do_get_inputs() const;
  virtual ModelObjectsTemp do_get_outputs() const;
  IMP_OBJECT_METHODS(PairsConstraint);
#endif
};

IMP_OBJECTS(PairsConstraint,PairsConstraints);


IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_PAIRS_CONSTRAINT_H */
