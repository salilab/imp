/**
 *  \file IMP/core/ClassnameConstraint.h
 *  \brief Use a ClassnameModifier applied to a PLURALVARIABLETYPE to
 *  maintain an invariant
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_CLASSNAME_CONSTRAINT_H
#define IMPCORE_CLASSNAME_CONSTRAINT_H

#include <IMP/core/core_config.h>
#include <IMP/internal/TupleConstraint.h>
#include <IMP/ClassnameModifier.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

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
                                          ClassnameModifier>
#endif
    {
  typedef IMP::internal::TupleConstraint<ClassnameModifier,
                                         ClassnameModifier> P;
  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<
                    IMP::internal::TupleConstraint<ClassnameModifier,
                                        ClassnameModifier> >(this));
  }
  IMP_OBJECT_SERIALIZE_DECL(ClassnameConstraint);

 public:
  ClassnameConstraint(ClassnameModifier *before,
                      ClassnameModifier *after, Model *m,
                      PASSINDEXTYPE vt,
                      std::string name = "ClassnameConstraint %1%",
                      bool can_skip=false)
      : IMP::internal::TupleConstraint<
            ClassnameModifier, ClassnameModifier>(before, after, m,
                                                  vt, name, can_skip) {}

  ClassnameConstraint() {}

  //! Get the ClassnameModifier object used in this constraint
  ClassnameModifier *get_before_modifier() const {
    return P::get_before_modifier();
  }

#if defined(IMP_DOXYGEN) || defined(SWIG)
  //! Get the index(es) used in this constraint
  typename ClassnameModifier::IndexArgument get_index() const;

 protected:
  void do_update_attributes();
  void do_update_derivatives(DerivativeAccumulator *da);
  virtual ModelObjectsTemp do_get_inputs() const;
  virtual ModelObjectsTemp do_get_outputs() const;
#endif
  IMP_OBJECT_METHODS(ClassnameConstraint);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CLASSNAME_CONSTRAINT_H */
