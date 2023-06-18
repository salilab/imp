/**
 *  \file generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_TUPLE_CONSTRAINT_H
#define IMPKERNEL_TUPLE_CONSTRAINT_H

#include "../Constraint.h"
#include "container_helpers.h"
#include <IMP/Pointer.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

template <class Before, class After>
class TupleConstraint : public Constraint {
  IMP::PointerMember<Before> f_;
  IMP::PointerMember<After> af_;
  typename Before::IndexArgument v_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Constraint>(this), f_, af_, v_);
  }

 public:
  TupleConstraint(Before *before, After *after, Model *m,
                  const typename Before::IndexArgument &vt,
                  std::string name = "TupleConstraint %1%",
                  bool can_skip=false);

  TupleConstraint() {}

  //! Apply this modifier to all the elements after an evaluate
  void set_after_evaluate_modifier(After *f) { af_ = f; }

  //! Apply this modifier to all the elements before an evaluate
  void set_before_evaluate_modifier(Before *f) { f_ = f; }

  typename Before::Argument get_argument() const {
    return get_particle(ScoreState::get_model(), v_);
  }

 protected:
  virtual void do_update_attributes() override;
  virtual void do_update_derivatives(DerivativeAccumulator *da) override;
  virtual ModelObjectsTemp do_get_inputs() const override;
  virtual ModelObjectsTemp do_get_outputs() const override;
  IMP_OBJECT_METHODS(TupleConstraint);
  ;
};

template <class Before, class After>
TupleConstraint<Before, After>::TupleConstraint(
    Before *before, After *after, Model *m,
    const typename Before::IndexArgument &vt, std::string name, bool can_skip)
    : Constraint(m, name), v_(vt) {
  if (before) f_ = before;
  if (after) af_ = after;
  set_can_skip(can_skip);
}

template <class Before, class After>
void TupleConstraint<Before, After>::do_update_attributes() {
  IMP_OBJECT_LOG;
  if (!f_) return;
  IMP_CHECK_OBJECT(f_);
  f_->apply_index(get_model(), v_);
}

template <class Before, class After>
void TupleConstraint<Before, After>::do_update_derivatives(
    DerivativeAccumulator *) {
  IMP_OBJECT_LOG;
  if (!af_) return;
  IMP_CHECK_OBJECT(af_);
  af_->apply_index(get_model(), v_);
}

template <class Before, class After>
ModelObjectsTemp TupleConstraint<Before, After>::do_get_inputs() const {
  ModelObjectsTemp ret;
  if (f_) {
    ret += f_->get_inputs(get_model(), flatten(v_));
    ret += f_->get_outputs(get_model(), flatten(v_));
  } else if (af_) {
    ret += af_->get_outputs(get_model(), flatten(v_));
  }
  return ret;
}

template <class Before, class After>
ModelObjectsTemp TupleConstraint<Before, After>::do_get_outputs() const {
  ModelObjectsTemp ret;
  if (f_) {
    ret += f_->get_outputs(get_model(), flatten(v_));
  } else if (af_) {
    ret += af_->get_inputs(get_model(), flatten(v_));
    ret += af_->get_outputs(get_model(), flatten(v_));
  }
  return ret;
}

template <class Before, class After>
inline Constraint *create_tuple_constraint(Before *b, After *a,
                                           const typename Before::Argument &t,
                                           std::string name = std::string(),
                                           bool can_skip=false) {
  if (name == std::string()) {
    if (b) name += " and  " + b->get_name();
    if (a) name += " and " + a->get_name();
  }
  return new internal::TupleConstraint<Before, After>(
                   b, a, internal::get_model(t), get_index(t), name, can_skip);
}

#ifndef IMP_DOXYGEN
template <class Before, class After>
inline Constraint *create_tuple_constraint(Pointer<Before> b,
                                           Pointer<After> a,
                                           const typename Before::Argument &t,
                                           std::string name = std::string(),
                                           bool can_skip=false) {
  return create_tuple_constraint<Before, After>(b.get(), a.get(), t, name,
                                                can_skip);
}
template <class Before, class After>
inline Constraint *create_tuple_constraint(Before *b, Pointer<After> a,
                                           const typename Before::Argument &t,
                                           std::string name = std::string(),
                                           bool can_skip=false) {
  return create_tuple_constraint<Before, After>(b, a.get(), t, name, can_skip);
}
template <class Before, class After>
inline Constraint *create_tuple_constraint(Pointer<Before> b, After *a,
                                           const typename Before::Argument &t,
                                           std::string name = std::string(),
                                           bool can_skip=false) {
  return create_tuple_constraint<Before, After>(b.get(), a, t, name, can_skip);
}
#endif

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_TUPLE_CONSTRAINT_H */
