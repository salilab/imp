/**
 *  \file core/generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_GENERIC_H
#define IMPCORE_GENERIC_H

#include "core_macros.h"
#include "core_config.h"
#include "XYZR.h"
#include <IMP/base_types.h>
#include <IMP/algebra/Segment3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/core/internal/singleton_helpers.h>
#include <IMP/core/internal/pair_helpers.h>
#include <IMP/core/internal/triplet_helpers.h>
#include <IMP/core/internal/quad_helpers.h>
#include <IMP/Model.h>
#include <IMP/Particle.h>

IMPCORE_BEGIN_NAMESPACE


/** When programming in C++, you can use TupleRestraint instead
    of a SingletonRestraint, PairRestraint, etc. The result is
    somewhat faster (20% or so). See
    \ref cppspecialization "specializing for speed" for more
    information.
*/
template <class Score>
class TupleRestraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
public Restraint
#else
public IMP::internal::SimpleRestraintParentTraits<Score>::SimpleRestraint
#endif
{
  IMP::internal::OwnerPointer<Score> ss_;
  typename Score::Argument v_;
  mutable double score_;
public:
  //! Create the restraint.
  /** This function takes the function to apply to the
      stored Groupname and the Groupname.
  */
  TupleRestraint(Score *ss,
                 const typename Score::Argument& vt,
                 std::string name="TupleRestraint %1%");

#if !defined(IMP_DOXYGEN)
  virtual Score* get_score() const {return ss_;}
  virtual typename Score::Argument get_argument() const {return v_;}
#endif

  IMP_INCREMENTAL_RESTRAINT(TupleRestraint);
};


/** \relatesalso TupleRestraint
 */
template <class Score>
Restraint* create_restraint(Score *s,
                            const typename Score::Argument &t,
                            std::string name= std::string()) {
    if (name==std::string()) {
      name= std::string("Restraint on ")+s->get_name();
  }
    return new TupleRestraint<Score>(s, t, name);
}




/** When programming in C++, you can use TupleConstraint instead
    of a SingletonConstraint, PairConstraint, etc. The result is
    somewhat faster (20% or so). See
    \ref cppspecialization "specializing for speed" for more
    information.
*/
template <class Before, class After>
class TupleConstraint : public Constraint
{
  IMP::internal::OwnerPointer<Before> f_;
  IMP::internal::OwnerPointer<After> af_;
  typename Before::Argument v_;
public:
  TupleConstraint(Before *before,
                  After *after,
                  const typename Before::Argument& vt,
                  std::string name="GroupnameConstraint %1%");

  //! Apply this modifier to all the elements after an evaluate
  void set_after_evaluate_modifier(After* f) {
    af_=f;
  }

  //! Apply this modifier to all the elements before an evaluate
  void set_before_evaluate_modifier(Before* f) {
    f_=f;
  }

  IMP_CONSTRAINT(TupleConstraint);
};



/** \relatesalso TupleConstraint
 */
template <class Before, class After>
Constraint* create_constraint(Before *b, After *a,
                              const typename Before::Argument &t,
                              std::string name=std::string()) {
  if (name==std::string()) {
    name= std::string("Constraint with ");
    if (b) name+= " and  "+b->get_name();
    if (a) name+= " and " +a->get_name();
  }
  return new TupleConstraint<Before, After>(b, a, t);
}


IMPCORE_END_NAMESPACE

#include "internal/generic_impl.h"

#endif  /* IMPCORE_GENERIC_H */
