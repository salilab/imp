/**
 *  \file IMP/core/ClassnameRestraint.h
 *  \brief Apply a ClassnameScore to a Classname.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_CLASSNAME_RESTRAINT_H
#define IMPCORE_CLASSNAME_RESTRAINT_H

#include <IMP/core/core_config.h>

#include <IMP/internal/TupleRestraint.h>
#include <IMP/ClassnameScore.h>

#include <iostream>

IMPCORE_BEGIN_NAMESPACE

//! Applies a ClassnameScore to a Classname.
/** This restraint stores a Classname.
    \see ClassnameRestraint
 */
class ClassnameRestraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
    public Restraint
#else
    public IMP::internal::TupleRestraint<ClassnameScore>
#endif
    {
 public:
  //! Create the restraint.
  /** This function takes the function to apply to the
      stored Classname and the Classname.
   */
  ClassnameRestraint(Model *m, ClassnameScore *ss, PASSINDEXTYPE vt,
                     std::string name = "ClassnameRestraint %1%")
      : IMP::internal::TupleRestraint<ClassnameScore>(ss, m, vt, name) {
  }

  //! Create the restraint.
  /** This function takes the function to apply to the
      stored Classname and the Classname.
      \deprecated_at{2.5} Use the index-based constructor instead.
   */
  IMPCORE_DEPRECATED_METHOD_DECL(2.5)
  ClassnameRestraint(ClassnameScore *ss, ARGUMENTTYPE vt,
                     std::string name = "ClassnameRestraint %1%")
      : IMP::internal::TupleRestraint<ClassnameScore>(
            ss, IMP::internal::get_model(vt),
            IMP::internal::get_index(vt), name) {
    IMPCORE_DEPRECATED_METHOD_DEF(2.5,
                                  "Use the index-based constructor instead.");
  }

#if defined(SWIG) || defined(IMP_DOXYGEN)
 protected:
  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  IMP::ModelObjectsTemp do_get_inputs() const;
  IMP_OBJECT_METHODS(ClassnameRestraint)
#endif
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CLASSNAME_RESTRAINT_H */
