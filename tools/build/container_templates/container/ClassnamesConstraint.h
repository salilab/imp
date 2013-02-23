/**
 *  \file IMP/container/ClassnamesConstraint.h
 *  \brief Use a ClassnameModifier applied to a PLURALVARIABLETYPE to
 *  maintain an invariant
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_CLASSNAMES_CONSTRAINT_H
#define IMPCONTAINER_CLASSNAMES_CONSTRAINT_H

#include <IMP/container/container_config.h>
#include <IMP/ClassnameContainer.h>
#include <IMP/ClassnameModifier.h>
#include <IMP/Constraint.h>
#include <IMP/score_state_macros.h>
#include <IMP/internal/ContainerConstraint.h>

IMPKERNEL_BEGIN_NAMESPACE
// for swig
class ClassnameContainer;
class ClassnameModifier;
IMPKERNEL_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE
//! Apply a ClassnameFunction to a ClassnameContainer to maintain an invariant
/** The score state is passed up to two ClassnameModifiers, one to
    apply before evaluation and the other after. The one after
    should take a DerivativeAccumulator as its last argument for
    ClassnameModifier::apply() and will only be called if
    the score was computed with derivatives.

    An example showing a how to use such a score state to maintain a cover
    of the atoms of a protein by a sphere per residue.
    \verbinclude cover_particles.py

    \see core::ClassnameConstraint
 */
class ClassnamesConstraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
 public Constraint
#else
 public IMP::kernel::internal::ContainerConstraint<ClassnameModifier,
                                           ClassnameDerivativeModifier,
                                           ClassnameContainer>
#endif
{
  typedef IMP::kernel::internal::ContainerConstraint<ClassnameModifier,
                                           ClassnameDerivativeModifier,
      ClassnameContainer> P;
public:
  /** \param[in] c The Container to hold the elements to process
      \param[in] before The ClassnameModifier to apply to all elements
      before evaluate.
      \param[in] after The ClassnameModifier to apply to all elements
      after evaluate.
      \param[in] name The object name
   */
  ClassnamesConstraint(ClassnameModifier *before,
                       ClassnameDerivativeModifier *after,
                       ClassnameContainerAdaptor c,
                       std::string name="ClassnameConstraint %1%"):
      P(before, after, c, name)
      {}
#if defined(IMP_DOXYGEN) || defined(SWIG)
protected:
  void do_update_attributes();
  void do_update_derivatives(DerivativeAccumulator *da);
  virtual ModelObjectsTemp do_get_inputs() const;
  virtual ModelObjectsTemp do_get_outputs() const;
  IMP_OBJECT_METHODS(ClassnamesConstraint);
#endif
};

IMP_OBJECTS(ClassnamesConstraint,ClassnamesConstraints);


IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_CLASSNAMES_CONSTRAINT_H */
