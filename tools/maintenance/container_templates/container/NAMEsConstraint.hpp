/**
 *  \file IMP/container/CLASSNAMEsConstraint.h
 *  \brief Use a CLASSNAMEModifier applied to a PLURALVARIABLETYPE to
 *  maintain an invariant
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_HEADERNAMES_CONSTRAINT_H
#define IMPCONTAINER_HEADERNAMES_CONSTRAINT_H

#include <IMP/container/container_config.h>
#include <IMP/CLASSNAMEContainer.h>
#include <IMP/CLASSNAMEModifier.h>
#include <IMP/Constraint.h>
#include <IMP/score_state_macros.h>
#include <IMP/internal/ContainerConstraint.h>

IMP_BEGIN_NAMESPACE
// for swig
class CLASSNAMEContainer;
class CLASSNAMEModifier;
IMP_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE
//! Apply a CLASSNAMEFunction to a CLASSNAMEContainer to maintain an invariant
/** The score state is passed up to two CLASSNAMEModifiers, one to
    apply before evaluation and the other after. The one after
    should take a DerivativeAccumulator as its last argument for
    CLASSNAMEModifier::apply() and will only be called if
    the score was computed with derivatives.

    An example showing a how to use such a score state to maintain a cover
    of the atoms of a protein by a sphere per residue.
    \verbinclude cover_particles.py

    \see core::CLASSNAMEConstraint
 */
class IMPCONTAINEREXPORT CLASSNAMEsConstraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
 public Constraint
#else
 public IMP::internal::ContainerConstraint<CLASSNAMEModifier,
                                           CLASSNAMEDerivativeModifier,
                                           CLASSNAMEContainer>
#endif
{
  typedef IMP::internal::ContainerConstraint<CLASSNAMEModifier,
                                           CLASSNAMEDerivativeModifier,
      CLASSNAMEContainer> P;
public:
  /** \param[in] c The Container to hold the elements to process
      \param[in] before The CLASSNAMEModifier to apply to all elements
      before evaluate.
      \param[in] after The CLASSNAMEModifier to apply to all elements
      after evaluate.
      \param[in] name The object name
   */
  CLASSNAMEsConstraint(CLASSNAMEModifier *before,
                       CLASSNAMEDerivativeModifier *after,
                       CLASSNAMEContainerAdaptor c,
                       std::string name="CLASSNAMEConstraint %1%"):
      P(before, after, c, name)
      {}
#if defined(IMP_DOXYGEN) || defined(SWIG)
protected:
  void do_update_attributes();
  void do_update_derivatives(DerivativeAccumulator *da);
  virtual ModelObjectsTemp do_get_inputs() const;
  virtual ModelObjectsTemp do_get_outputs() const;
  IMP_OBJECT_METHODS(CLASSNAMEsConstraint);
#endif
};

IMP_OBJECTS(CLASSNAMEsConstraint,CLASSNAMEsConstraints);


IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_HEADERNAMES_CONSTRAINT_H */
