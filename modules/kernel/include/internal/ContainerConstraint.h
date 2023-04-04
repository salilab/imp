/**
 *  \file ContainerConstraint.h
 *  \brief Templated and more efficient constraint implementation.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_CONTAINER_CONSTRAINT_H
#define IMPKERNEL_CONTAINER_CONSTRAINT_H

#include <IMP/kernel_config.h>
#include "../base_types.h"
#include "../Constraint.h"

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

/** Create a constraint tied to particular modifiers and containers. This
    functionality, which is only available in C++, can result in faster
    evaluates.
*/
template <class Before, class After, class Container>
class ContainerConstraint : public Constraint {
  IMP::PointerMember<Before> f_;
  IMP::PointerMember<After> af_;
  IMP::PointerMember<Container> c_;

 public:
  ContainerConstraint(Before *before, After *after, Container *c,
                      std::string name="ContainerConstraint %1%",
                      bool can_skip=false);

  //! Apply this modifier to all the elements after an evaluate
  void set_after_evaluate_modifier(After *f) { af_ = f; }

  //! Apply this modifier to all the elements before an evaluate
  void set_before_evaluate_modifier(Before *f) { f_ = f; }

  // only report actual interactions
  ModelObjectsTemps do_get_interactions() const override {
    ModelObjectsTemps ret;
    typename Container::ContainedIndexTypes ps = c_->get_range_indexes();
    for (unsigned int i = 0; i < ps.size(); ++i) {
      {
        ModelObjectsTemp cur;
        cur += IMP::get_particles(get_model(), flatten(ps[i]));
        cur.push_back(c_);
        ret.push_back(cur);
      }
      if (f_) {
        ModelObjectsTemp ip = f_->get_inputs(get_model(), flatten(ps[i]));
        ModelObjectsTemp op = f_->get_outputs(get_model(), flatten(ps[i]));
        ret.push_back(ip + op);
      } else {
        ModelObjectsTemp ip = af_->get_inputs(get_model(), flatten(ps[i]));
        ModelObjectsTemp op = af_->get_outputs(get_model(), flatten(ps[i]));
        ret.push_back(ip + op);
      }
      std::sort(ret.back().begin(), ret.back().end());
      ret.back().erase(std::unique(ret.back().begin(), ret.back().end()),
                       ret.back().end());
    }
    return ret;
  }

 protected:
  virtual void do_update_attributes() override;
  virtual void do_update_derivatives(DerivativeAccumulator *da) override;
  virtual ModelObjectsTemp do_get_inputs() const override;
  virtual ModelObjectsTemp do_get_outputs() const override;
  IMP_OBJECT_METHODS(ContainerConstraint);
  ;
};

/** See ContainerConstraint

    Helper to create a ContainerConstraint.
 */
template <class Before, class After, class Container>
inline Constraint *create_container_constraint(Before *b, After *a,
                                               Container *c,
                                               std::string name=std::string(),
                                               bool can_skip=false) {
  if (name == std::string()) {
    if (b) name += " and  " + b->get_name();
    if (a) name += " and " + a->get_name();
  }
  return new ContainerConstraint<Before, After, Container>(b, a, c, name,
                                                           can_skip);
}

template <class Before, class After, class C>
ContainerConstraint<Before, After, C>::ContainerConstraint(Before *before,
                                                           After *after, C *c,
                                                           std::string name,
                                                           bool can_skip)
    : Constraint(c->get_model(), name), c_(c) {
  if (before) f_ = before;
  if (after) af_ = after;
  set_can_skip(can_skip);
}

template <class Before, class After, class C>
void ContainerConstraint<Before, After, C>::do_update_attributes() {
  IMP_OBJECT_LOG;
  if (!f_) return;
  IMP_CHECK_OBJECT(f_);
  IMP_CHECK_OBJECT(c_);
  c_->apply_generic(f_.get());
}

template <class Before, class After, class C>
void ContainerConstraint<Before, After, C>::do_update_derivatives(
    DerivativeAccumulator *da) {
  IMP_OBJECT_LOG;
  if (!af_ || !da) return;
  IMP_CHECK_OBJECT(af_);
  IMP_CHECK_OBJECT(c_);
  c_->apply_generic(af_.get());
}

template <class Before, class After, class C>
ModelObjectsTemp ContainerConstraint<Before, After, C>::do_get_inputs() const {
  ModelObjectsTemp ret;
  if (f_) {
    ret += f_->get_inputs(get_model(), c_->get_all_possible_indexes());

    ret += f_->get_outputs(get_model(), c_->get_all_possible_indexes());
  } else {
    ret = af_->get_outputs(get_model(), c_->get_all_possible_indexes());
  }
  ret.push_back(c_);
  return ret;
}

template <class Before, class After, class C>
ModelObjectsTemp ContainerConstraint<Before, After, C>::do_get_outputs() const {
  ModelObjectsTemp ret;
  if (f_) {
    ret += f_->get_outputs(get_model(), c_->get_all_possible_indexes());
  } else {
    ret = af_->get_inputs(get_model(), c_->get_all_possible_indexes());
    ret = af_->get_outputs(get_model(), c_->get_all_possible_indexes());
  }
  return ret;
}

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_CONTAINER_CONSTRAINT_H */
