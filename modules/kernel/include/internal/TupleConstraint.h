/**
 *  \file generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_TUPLE_CONSTRAINT_H
#define IMPKERNEL_TUPLE_CONSTRAINT_H

#include "../Constraint.h"
#include "container_helpers.h"
#include "singleton_helpers.h"
#include "pair_helpers.h"
#include "triplet_helpers.h"
#include "quad_helpers.h"
#include "../score_state_macros.h"

IMP_BEGIN_INTERNAL_NAMESPACE

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
    IMP_IF_CHECK(base::USAGE) {
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
    IMP_IF_CHECK(base::USAGE) {
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

template <class Before, class After>
inline Constraint* create_constraint(Before *b, After *a,
                              const typename Before::Argument &t,
                              std::string name=std::string()) {
  if (name==std::string()) {
    if (b) name+= " and  "+b->get_name();
    if (a) name+= " and " +a->get_name();
  }
  return new internal::TupleConstraint<Before, After>(b, a, t, name);
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

#endif  /* IMPKERNEL_TUPLE_CONSTRAINT_H */
