/**
 *  \file IMP/generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_GENERIC_H
#define IMPKERNEL_GENERIC_H

#include "macros.h"
#include "kernel_config.h"
#include "SingletonScore.h"
#include "PairScore.h"
#include "TripletScore.h"
#include "QuadScore.h"
#include "Constraint.h"
#include "Restraint.h"
#include "internal/container_helpers.h"
#include "internal/scoring_functions.h"
#include "internal/singleton_helpers.h"
#include "internal/pair_helpers.h"
#include "internal/triplet_helpers.h"
#include "internal/quad_helpers.h"
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>

IMP_BEGIN_NAMESPACE



/** Create a ScoringFunction on a single restraints.*/
template <class RestraintType>
inline ScoringFunction* create_scoring_function(RestraintType* rs,
                                               double weight=1.0,
                                                double max=NO_MAX) {
  if (dynamic_cast<RestraintSet*>(rs)) {
    RestraintSet *rrs=dynamic_cast<RestraintSet*>(rs);
    if (rrs->get_number_of_restraints()==0) {
      // ick
      return new RestraintsScoringFunction(RestraintsTemp(1,rs), weight, max);
    }
    return new RestraintsScoringFunction(RestraintsTemp(rrs->restraints_begin(),
                                                        rrs->restraints_end()),
                                         weight*rs->get_weight(),
                                         std::min(max,
                                                  rs->get_maximum_score()));
  } else {
    if (weight==1.0 && max==NO_MAX) {
      return new internal::RestraintScoringFunction<RestraintType>(rs);
    } else {
      return new internal::WrappedRestraintScoringFunction<RestraintType>(rs,
                                                                        weight,
                                                                          max);
    }
  }
}



#ifndef IMP_DOXYGEN
template <class Score>
class TupleRestraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
public Restraint
#else
public IMP::internal::SimpleRestraintParentTraits<Score>::SimpleRestraint
#endif
{
  BOOST_STATIC_ASSERT(!(boost::is_same<Score, SingletonScore>::value));
  BOOST_STATIC_ASSERT(!(boost::is_same<Score, PairScore>::value));
  BOOST_STATIC_ASSERT(!(boost::is_same<Score, TripletScore>::value));
  BOOST_STATIC_ASSERT(!(boost::is_same<Score, QuadScore>::value));
  IMP::OwnerPointer<Score> ss_;
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

  IMP_RESTRAINT(TupleRestraint);
};
#endif

/** When programming in C++, you can use generic restraint instead
    of a SingletonRestraint, PairRestraint, etc. The result is
    somewhat faster (20% or so). See
    \ref cppspecialization "specializing for speed" for more
    information.
*/
template <class Score>
inline Restraint* create_restraint(Score *s,
                            const typename Score::Argument &t,
                            std::string name= std::string()) {
  if (name==std::string()) {
      name= std::string("Restraint on ")+s->get_name();
  }
    return new TupleRestraint<Score>(s, t, name);
}

template <class Score>
inline Restraint* create_restraint(const Score *s,
                            const typename Score::Argument &t,
                            std::string name= std::string()) {
    if (name==std::string()) {
      std::ostringstream oss;
      oss << s->get_name() << " on " << t;
      name= oss.str();
  }
    return new TupleRestraint<Score>(const_cast<Score*>(s), t, name);
}


#ifndef IMP_DOXYGEN
template <class Before, class After>
class TupleConstraint : public Constraint
{
  IMP::OwnerPointer<Before> f_;
  IMP::OwnerPointer<After> af_;
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
#endif


/** When programming in C++, you can use generic constraint instead
    of a SingletonConstraint, PairConstraint, etc. The result is
    somewhat faster (20% or so). See
    \ref cppspecialization "specializing for speed" for more
    information.
*/
template <class Before, class After>
inline Constraint* create_constraint(Before *b, After *a,
                              const typename Before::Argument &t,
                              std::string name=std::string()) {
  if (name==std::string()) {
    if (b) name+= " and  "+b->get_name();
    if (a) name+= " and " +a->get_name();
  }
  return new TupleConstraint<Before, After>(b, a, t, name);
}


IMP_END_NAMESPACE

#include "internal/generic_impl.h"

#endif  /* IMPKERNEL_GENERIC_H */
