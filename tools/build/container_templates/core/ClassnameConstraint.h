/**
 *  \file IMP/core/ClassnameConstraint.h
 *  \brief Use a ClassnameModifier applied to a PLURALVARIABLETYPE to
 *  maintain an invariant
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_CLASSNAME_CONSTRAINT_H
#define IMPCORE_CLASSNAME_CONSTRAINT_H

#include <IMP/core/core_config.h>
#include <IMP/internal/TupleConstraint.h>
#include <IMP/ClassnameModifier.h>
#include <IMP/ClassnameDerivativeModifier.h>

IMPCORE_BEGIN_NAMESPACE
//! Apply a ClassnameFunction to a Classname
/** The score state is passed up to two ClassnameModifiers, one to
    apply before evaluation and the other after. The one after
    should take a DerivativeAccumulator as its last argument for
    ClassnameModifier::apply() and will only be called if
    the score was computed with derivatives.

    \see container::ClassnamesConstraint
 */
class ClassnameConstraint :
#if defined(IMP_DOXYGEN) || defined(SWIG)
    public Constraint
#else
    public IMP::internal::TupleConstraint<ClassnameModifier,
                                                  ClassnameDerivativeModifier>
#endif
    {
 public:
  /** \deprecated_at{2.1} Use the model/index constructor.
   */
  IMPCORE_DEPRECATED_METHOD_DECL(2.1)
  ClassnameConstraint(ClassnameModifier *before,
                      ClassnameDerivativeModifier *after, ARGUMENTTYPE vt,
                      std::string name = "ClassnameConstraint %1%")
      : IMP::internal::TupleConstraint<
            ClassnameModifier, ClassnameDerivativeModifier>(before, after, vt,
                                                            name) {
    IMPCORE_DEPRECATED_METHOD_DEF(2.1, "Use the model/index constructor.");
  }

  ClassnameConstraint(ClassnameModifier *before,
                      ClassnameDerivativeModifier *after, Model *m,
                      PASSINDEXTYPE vt,
                      std::string name = "ClassnameConstraint %1%",
                      bool can_skip=false)
      : IMP::internal::TupleConstraint<
            ClassnameModifier, ClassnameDerivativeModifier>(before, after, m,
                                                            vt, name,
                                                            can_skip) {}

#if defined(IMP_DOXYGEN) || defined(SWIG)
 protected:
  void do_update_attributes();
  void do_update_derivatives(DerivativeAccumulator *da);
  virtual ModelObjectsTemp do_get_inputs() const;
  virtual ModelObjectsTemp do_get_outputs() const;
  IMP_OBJECT_METHODS(ClassnameConstraint);
#endif
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CLASSNAME_CONSTRAINT_H */
