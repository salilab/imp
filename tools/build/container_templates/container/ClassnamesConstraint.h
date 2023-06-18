/**
 *  \file IMP/container/ClassnamesConstraint.h
 *  \brief Use a ClassnameModifier applied to a PLURALVARIABLETYPE to
 *  maintain an invariant
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_CLASSNAMES_CONSTRAINT_H
#define IMPCONTAINER_CLASSNAMES_CONSTRAINT_H

#include <IMP/container/container_config.h>
#include <IMP/ClassnameContainer.h>
#include <IMP/ClassnameModifier.h>
#include <IMP/Constraint.h>
#include <IMP/object_macros.h>
#include <IMP/internal/ContainerConstraint.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

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

    \see core::ClassnameConstraint
 */
class IMPCONTAINEREXPORT ClassnamesConstraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
    public Constraint
#else
    public IMP::internal::ContainerConstraint<
        ClassnameModifier, ClassnameModifier, ClassnameContainer>
#endif
    {
  typedef IMP::internal::ContainerConstraint<
      ClassnameModifier, ClassnameModifier, ClassnameContainer> P;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<P>(this));
  }

  IMP_OBJECT_SERIALIZE_DECL(ClassnamesConstraint);

 public:
  /** \param[in] c The Container to hold the elements to process
      \param[in] before The ClassnameModifier to apply to all elements
      before evaluate.
      \param[in] after The ClassnameModifier to apply to all elements
      after evaluate.
      \param[in] name The object name
   */
  ClassnamesConstraint(ClassnameModifier *before, ClassnameModifier *after,
                       ClassnameContainerAdaptor c,
                       std::string name = "ClassnamesConstraint %1%")
      : P(before, after, c, name) {}

  ClassnamesConstraint() {}

#if defined(IMP_DOXYGEN) || defined(SWIG)
 protected:
  void do_update_attributes();
  void do_update_derivatives(DerivativeAccumulator *da);
  virtual ModelObjectsTemp do_get_inputs() const;
  virtual ModelObjectsTemp do_get_outputs() const;
  IMP_OBJECT_METHODS(ClassnamesConstraint);
#endif
};

IMP_OBJECTS(ClassnamesConstraint, ClassnamesConstraints);

IMPCONTAINER_END_NAMESPACE

#endif /* IMPCONTAINER_CLASSNAMES_CONSTRAINT_H */
