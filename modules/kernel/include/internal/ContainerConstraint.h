/**
 *  \file core/generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_CONTAINER_CONSTRAINT_H
#define IMPKERNEL_CONTAINER_CONSTRAINT_H

#include "../kernel_config.h"
#include "../base_types.h"
#include "../Constraint.h"
#include "../score_state_macros.h"

IMP_BEGIN_INTERNAL_NAMESPACE


/** Create a constraint tied to particular modifiers and contains. This
    functionality, which is only available in C++ can result in faster
    evaluates.
*/
template <class Before, class After, class Container>
class ContainerConstraint : public Constraint
{
  IMP::OwnerPointer<Before> f_;
  IMP::OwnerPointer<After> af_;
  IMP::OwnerPointer<Container> c_;
public:
  ContainerConstraint(Before *before,
                      After *after, Container *c,
                       std::string name="ContainerConstraint %1%");

  //! Apply this modifier to all the elements after an evaluate
  void set_after_evaluate_modifier(After* f) {
    af_=f;
  }

  //! Apply this modifier to all the elements before an evaluate
  void set_before_evaluate_modifier(Before* f) {
    f_=f;
  }

  IMP_CONSTRAINT(ContainerConstraint);
};


/** \relatesalso ContainerConstraint

    Helper to create a ContainerConstraint.
 */
template <class Before, class After, class Container>
inline Constraint *create_container_constraint(Before *b, After *a,
                                               Container *c,
                              std::string name=std::string()) {
  if (name==std::string()) {
    if (b) name+= " and  "+b->get_name();
    if (a) name+= " and " +a->get_name();
  }
  return new ContainerConstraint<Before, After, Container>(b, a, c,
                                                           name);
}




template <class Before, class After, class C>
ContainerConstraint< Before, After, C>::ContainerConstraint(
                                         Before *before,
                                         After *after,
                                         C *c,
                                         std::string name):
  Constraint(name), c_(c) {
  if (before) f_=before;
  if (after) af_=after;
}

template <class Before, class After, class C>
void ContainerConstraint<Before, After, C>::do_update_attributes()
{
  IMP_OBJECT_LOG;
  if (!f_) return;
  IMP_CHECK_OBJECT(f_);
  IMP_CHECK_OBJECT(c_);
  if (c_->get_provides_access()) {
    f_->Before::apply_indexes(get_model(), c_->get_access());
  } else {
    c_->template_apply(f_.get());
  }
}

template <class Before, class After, class C>
void ContainerConstraint<Before, After, C>
::do_update_derivatives(DerivativeAccumulator *da)
{
  IMP_OBJECT_LOG;
  if (!af_) return;
  IMP_CHECK_OBJECT(af_);
  IMP_CHECK_OBJECT(c_);
  if (c_->get_provides_access()) {
    af_->After::apply_indexes(get_model(), c_->get_access(), *da);
  } else {
    c_->template_apply(af_.get(), *da);
  }
}


template <class Before, class After, class C>
ContainersTemp ContainerConstraint<Before, After, C>
::get_input_containers() const {
  return ContainersTemp(1, c_);
}

template <class Before, class After, class C>
ContainersTemp ContainerConstraint<Before, After, C>
::get_output_containers() const {
  return ContainersTemp();
}

template <class Before, class After, class C>
ParticlesTemp ContainerConstraint<Before, After, C>
::get_input_particles() const {
  ParticlesTemp ret;
  if (f_) {
    ret= IMP::internal::get_input_particles(f_.get(),
                                            c_->get_all_possible_particles());
    ParticlesTemp o= IMP::internal::get_output_particles(f_.get(),
                                            c_->get_all_possible_particles());
    ret.insert(ret.end(), o.begin(), o.end());
    IMP_IF_CHECK(USAGE) {
      if (af_) {
        ParticlesTemp oret
            = IMP::internal::get_output_particles(af_.get(),
                                            c_->get_all_possible_particles());
        std::sort(ret.begin(), ret.end());
        std::sort(oret.begin(), oret.end());
        ParticlesTemp t;
        std::set_union(ret.begin(), ret.end(), oret.begin(), oret.end(),
                       std::back_inserter(t));
        IMP_USAGE_CHECK(t.size() == ret.size(), "The particles written by "
                        << " the after modifier in " << get_name()
                        << " must be a subset of those read by the before "
                        << "modifier. Before: " << ret
                        << " and after " << oret);
      }
    }
  } else {
    ret= IMP::internal::get_output_particles(af_.get(),
                                          c_->get_all_possible_particles());
  }
  return ret;
}

template <class Before, class After, class C>
ParticlesTemp ContainerConstraint<Before, After, C>
::get_output_particles() const {
  ParticlesTemp ret;
  if (f_) {
    ret= IMP::internal::get_output_particles(f_.get(),
                                       c_->get_all_possible_particles());
    IMP_IF_CHECK(USAGE) {
      if (af_) {
        ParticlesTemp oret
            = IMP::internal::get_input_particles(af_.get(),
                                           c_->get_all_possible_particles());
        ParticlesTemp iret
            =IMP::internal::get_input_particles(f_.get(),
                                          c_->get_all_possible_particles());
        iret.insert(iret.end(), ret.begin(), ret.end());
        std::sort(iret.begin(), iret.end());
        std::sort(oret.begin(), oret.end());
        ParticlesTemp t;
        std::set_union(iret.begin(), iret.end(), oret.begin(), oret.end(),
                       std::back_inserter(t));
        IMP_USAGE_CHECK(t.size() == iret.size(), "The particles read by "
                      << " the after modifier in " << get_name() << " must "
                        << "be a subset of those written by the before "
                        << "modifier.");
      }
    }
  } else {
    ret= IMP::internal::get_input_particles(af_.get(),
                                           c_->get_all_possible_particles());
  }
  return ret;
}

template <class Before, class After, class C>
void ContainerConstraint<Before, After, C>
::do_show(std::ostream &out) const {
  out << "on " << *c_ << std::endl;
  if (f_) out << "before " << *f_ << std::endl;
  if (af_) out << "after " << *af_ << std::endl;
}




IMP_END_INTERNAL_NAMESPACE


#endif  /* IMPKERNEL_CONTAINER_CONSTRAINT_H */
