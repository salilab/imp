/**
 *  \file generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_GENERIC_IMPL_H
#define IMPKERNEL_GENERIC_IMPL_H

#include "../Constraint.h"
#include "../Restraint.h"

#include "container_helpers.h"
#include "singleton_helpers.h"
#include "pair_helpers.h"
#include "triplet_helpers.h"
#include "quad_helpers.h"

IMP_BEGIN_INTERNAL_NAMESPACE

/** Create a ScoringFunction on a single restraints.*/
template <class RestraintType>
inline ScoringFunction* create_scoring_function(RestraintType* rs,
                                               double weight=1.0,
                                                double max=NO_MAX,
                                                std::string name=
                                                std::string()) {
  if (name.empty()) {
    name= rs->get_name()+"ScoringFunction";
  }
  if (dynamic_cast<RestraintSet*>(rs)) {
    RestraintSet *rrs=dynamic_cast<RestraintSet*>(rs);
    if (rrs->get_number_of_restraints()==0) {
      // ick
      return new RestraintsScoringFunction(RestraintsTemp(1,rs), weight, max,
                                           name);
    }
    return new RestraintsScoringFunction(RestraintsTemp(rrs->restraints_begin(),
                                                        rrs->restraints_end()),
                                         weight*rs->get_weight(),
                                         std::min(max,
                                                  rs->get_maximum_score()),
                                         name);
  } else {
    if (weight==1.0 && max==NO_MAX) {
      return new internal::RestraintScoringFunction<RestraintType>(rs,
                                                                   name);
    } else {
      return new internal::WrappedRestraintScoringFunction<RestraintType>(rs,
                                                                        weight,
                                                                          max,
                                                                          name);
    }
  }
}


template <class Score>
class TupleRestraint : public Restraint
{
  IMP::OwnerPointer<Score> ss_;
  typename Score::IndexArgument v_;
public:
  //! Create the restraint.
  /** This function takes the function to apply to the
      stored Groupname and the Groupname.
  */
  TupleRestraint(Score *ss,
                 const typename Score::Argument& vt,
                 std::string name="TupleRestraint %1%");

  Score* get_score() const {return ss_;}
  typename Score::Argument get_argument() const {
    return get_particle(Restraint::get_model(),
                                  v_);
  }

  IMP_RESTRAINT(TupleRestraint);
};

template <class Before, class After>
class TupleConstraint : public Constraint
{
  IMP::OwnerPointer<Before> f_;
  IMP::OwnerPointer<After> af_;
  typename Before::IndexArgument v_;
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

  typename Before::Argument get_argument() const {
    return get_particle(ScoreState::get_model(),
                        v_);
  }

  IMP_CONSTRAINT(TupleConstraint);
};

template <class Score>
TupleRestraint<Score>
::TupleRestraint(Score *ss,
                 const typename Score::Argument& vt,
                 std::string name):
  Restraint(name),
  ss_(ss),
  v_(get_index(vt))
{
}

template <class Score>
double TupleRestraint<Score>
::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(ss_);
  return call_evaluate_index(Restraint::get_model(), ss_.get(), v_, accum);
}


template <class Score>
ParticlesTemp TupleRestraint<Score>::get_input_particles() const
{
  return IMP::internal::get_input_particles(ss_.get(),
                                            get_argument());
}

template <class Score>
ContainersTemp TupleRestraint<Score>::get_input_containers() const
{
  return IMP::internal::get_input_containers(ss_.get(),
                                             get_argument());
}

template <class Score>
void TupleRestraint<Score>::do_show(std::ostream& out) const
{
  out << "score " << *ss_ << std::endl;
  out << "data " << Showable(get_argument()) << std::endl;
}
template <class Score>
inline Restraint* create_restraint(Pointer<Score> s,
                            const typename Score::Argument &t,
                            std::string name= std::string()) {
  return create_restraint<Score>(s.get(), t, name);
}











template <class Before, class After>
TupleConstraint<Before, After>::TupleConstraint(Before *before,
                                                After *after,
                               const typename Before::Argument& vt,
                                                std::string name):
  Constraint(name), v_(get_index(vt)){
  if (before) f_=before;
  if (after) af_=after;
}

template <class Before, class After>
void TupleConstraint<Before, After>::do_update_attributes()
{
  IMP_OBJECT_LOG;
  if (!f_) return;
  IMP_LOG(TERSE, "Begin GroupnamesConstraint::update" << std::endl);
  IMP_CHECK_OBJECT(f_);
  call_apply_index(get_model(), f_.get(), v_);
  IMP_LOG(TERSE, "End GroupnamesConstraint::update" << std::endl);
}

template <class Before, class After>
void TupleConstraint<Before, After>
::do_update_derivatives(DerivativeAccumulator *da)
{
  IMP_OBJECT_LOG;
  if (!af_) return;
  IMP_LOG(TERSE, "Begin GroupnamesConstraint::after_evaluate" << std::endl);
  IMP_CHECK_OBJECT(af_);
  call_apply_index(get_model(), af_.get(), v_, *da);
  IMP_LOG(TERSE, "End GroupnamesConstraint::after_evaluate" << std::endl);
}

template <class Before, class After>
ContainersTemp TupleConstraint<Before, After>::get_input_containers() const {
  ContainersTemp ret;
  if (f_) {
    ret= IMP::internal::get_input_containers(f_.get(),
                                             get_argument());
  } else if (af_) {
    ret= IMP::internal::get_input_containers(af_.get(), get_argument());
  }
  return ret;
}

template <class Before, class After>
ContainersTemp TupleConstraint<Before, After>::get_output_containers() const {
  ContainersTemp ret;
  if (f_) {
    ret= IMP::internal::get_output_containers(f_.get(), get_argument());
  } else if (af_) {
    ret= IMP::internal::get_output_containers(af_.get(), get_argument());
  }
  return ret;
}

template <class Before, class After>
ParticlesTemp TupleConstraint<Before, After>::get_input_particles() const {
  ParticlesTemp ret;
  if (f_) {
    ret= IMP::internal::get_input_particles(f_.get(), get_argument());
    ParticlesTemp o= IMP::internal::get_output_particles(f_.get(),
                                                         get_argument());
    ret.insert(ret.end(), o.begin(), o.end());
    IMP_IF_CHECK(USAGE) {
      if (af_) {
        ParticlesTemp oret= IMP::internal::get_input_particles(af_.get(),
                                                               get_argument());
        std::sort(ret.begin(), ret.end());
        std::sort(oret.begin(), oret.end());
        ParticlesTemp t;
        std::set_union(ret.begin(), ret.end(), oret.begin(), oret.end(),
                       std::back_inserter(t));
        IMP_USAGE_CHECK(t.size() == ret.size(),
                        "The particles written by "
                        << " the after modifier in " << get_name() << " must "
                        << "be a subset of those read by the before "
                        << "modifier. Before: " << ret
                        << " and after " << oret);
      }
    }
  } else {
    ret=IMP::internal::get_output_particles(af_.get(), get_argument());
  }
  return ret;
}

template <class Before, class After>
ParticlesTemp TupleConstraint<Before, After>::get_output_particles() const {
  ParticlesTemp ret;
  if (f_) {
    ret= IMP::internal::get_output_particles(f_.get(), get_argument());
    IMP_IF_CHECK(USAGE) {
      if (af_) {
        ParticlesTemp oret= IMP::internal::get_input_particles(af_.get(),
                                                get_argument());
        ParticlesTemp iret= IMP::internal::get_input_particles(f_.get(),
                                                get_argument());
        iret.insert(iret.end(), ret.begin(), ret.end());
        std::sort(iret.begin(), iret.end());
        std::sort(oret.begin(), oret.end());
        ParticlesTemp t;
        std::set_union(iret.begin(), iret.end(), oret.begin(), oret.end(),
                       std::back_inserter(t));
        IMP_USAGE_CHECK(t.size() == iret.size(), "The particles read by "
                      << " the after modifier in " << get_name() << " must "
                        << "be a subset of those written by the before"
                        << " modifier.");
      }
    }
  } else {
    ret= IMP::internal::get_input_particles(af_.get(), get_argument());
  }
  return ret;
}

template <class Before, class After>
void TupleConstraint<Before, After>::do_show(std::ostream &out) const {
  out << "on " << Showable(get_particle(ScoreState::get_model(),
                                        v_)) << std::endl;
  if (f_) out << "before " << *f_ << std::endl;
  if (af_) out << "after " << *af_ << std::endl;
}

#ifndef IMP_DOXYGEN
template <class Before, class After>
inline Constraint* create_constraint(Pointer<Before> b, Pointer<After> a,
                              const typename Before::Argument &t,
                              std::string name=std::string()) {
  return create_constraint<Before, After>(b.get(), a.get(), t, name);
}
template <class Before, class After>
inline Constraint* create_constraint(Before* b, Pointer<After> a,
                              const typename Before::Argument &t,
                              std::string name=std::string()) {
  return create_constraint<Before, After>(b, a.get(), t, name);
}
template <class Before, class After>
inline Constraint* create_constraint(Pointer<Before> b, After* a,
                              const typename Before::Argument &t,
                              std::string name=std::string()) {
  return create_constraint<Before, After>(b.get(), a, t, name);
}
#endif

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_GENERIC_IMPL_H */
