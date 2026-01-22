/**
 *  \file IMP/core/ClassnameRestraint.h
 *  \brief Apply a ClassnameScore to a Classname.
 *
 *  Copyright 2007-2025 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_CLASSNAME_RESTRAINT_H
#define IMPCORE_CLASSNAME_RESTRAINT_H

#include <IMP/core/core_config.h>

#include <IMP/internal/TupleRestraint.h>
#include <IMP/ClassnameScore.h>

#include <iostream>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

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
  typedef IMP::internal::TupleRestraint<ClassnameScore> P;
  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<
                    IMP::internal::TupleRestraint<ClassnameScore> >(this));
  }
  IMP_OBJECT_SERIALIZE_DECL(ClassnameRestraint);
 public:
  //! Create the restraint.
  /** This function takes the function to apply to the
      stored Classname and the Classname.
   */
  ClassnameRestraint(Model *m, ClassnameScore *ss, PASSINDEXTYPE vt,
                     std::string name = "ClassnameRestraint %1%")
      : IMP::internal::TupleRestraint<ClassnameScore>(ss, m, vt, name) {
  }
  ClassnameRestraint() {}

  //! Get the ClassnameScore object used in this restraint
  ClassnameScore *get_score_object() const { return P::get_score(); }

#if defined(SWIG) || defined(IMP_DOXYGEN)
  //! Get the index(es) used in this restraint
  typename ClassnameScore::IndexArgument get_index() const;

 protected:
  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  IMP::ModelObjectsTemp do_get_inputs() const;
#endif
  IMP_OBJECT_METHODS(ClassnameRestraint)
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CLASSNAME_RESTRAINT_H */
