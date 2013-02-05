/**
 *  \file IMP/container/SingletonsConstraint.h
 *  \brief Use a SingletonModifier applied to a ParticlesTemp to
 *  maintain an invariant
 *
 *  WARNING This file was generated from NAMEsConstraint.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_SINGLETONS_CONSTRAINT_H
#define IMPCONTAINER_SINGLETONS_CONSTRAINT_H

#include <IMP/container/container_config.h>
#include <IMP/SingletonContainer.h>
#include <IMP/SingletonModifier.h>
#include <IMP/Constraint.h>
#include <IMP/score_state_macros.h>
#include <IMP/internal/ContainerConstraint.h>

IMPKERNEL_BEGIN_NAMESPACE
// for swig
class SingletonContainer;
class SingletonModifier;
IMPKERNEL_END_NAMESPACE

IMPCONTAINER_BEGIN_NAMESPACE
//! Apply a SingletonFunction to a SingletonContainer to maintain an invariant
/** The score state is passed up to two SingletonModifiers, one to
    apply before evaluation and the other after. The one after
    should take a DerivativeAccumulator as its last argument for
    SingletonModifier::apply() and will only be called if
    the score was computed with derivatives.

    An example showing a how to use such a score state to maintain a cover
    of the atoms of a protein by a sphere per residue.
    \verbinclude cover_particles.py

    \see core::SingletonConstraint
 */
class IMPCONTAINEREXPORT SingletonsConstraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
 public Constraint
#else
 public IMP::kernel::internal::ContainerConstraint<SingletonModifier,
                                           SingletonDerivativeModifier,
                                           SingletonContainer>
#endif
{
  typedef IMP::kernel::internal::ContainerConstraint<SingletonModifier,
                                           SingletonDerivativeModifier,
      SingletonContainer> P;
public:
  /** \param[in] c The Container to hold the elements to process
      \param[in] before The SingletonModifier to apply to all elements
      before evaluate.
      \param[in] after The SingletonModifier to apply to all elements
      after evaluate.
      \param[in] name The object name
   */
  SingletonsConstraint(SingletonModifier *before,
                       SingletonDerivativeModifier *after,
                       SingletonContainerAdaptor c,
                       std::string name="SingletonConstraint %1%"):
      P(before, after, c, name)
      {}
#if defined(IMP_DOXYGEN) || defined(SWIG)
protected:
  void do_update_attributes();
  void do_update_derivatives(DerivativeAccumulator *da);
  virtual ModelObjectsTemp do_get_inputs() const;
  virtual ModelObjectsTemp do_get_outputs() const;
  IMP_OBJECT_METHODS(SingletonsConstraint);
#endif
};

IMP_OBJECTS(SingletonsConstraint,SingletonsConstraints);


IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_SINGLETONS_CONSTRAINT_H */
