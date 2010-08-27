/**
 *  \file core/generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_GENERIC_IMPL_H
#define IMPCORE_GENERIC_IMPL_H


IMPCORE_BEGIN_NAMESPACE


template <class Score>
TupleRestraint<Score>
::TupleRestraint(Score *ss,
                 const typename Score::Argument& vt,
                 std::string name):
  Restraint(name),
  ss_(ss),
  v_(vt),
  score_(std::numeric_limits<double>::quiet_NaN())
{
}

template <class Score>
double TupleRestraint<Score>
::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(ss_);
  score_ = ss_->Score::evaluate(v_, accum);

  return score_;
}

template <class Score>
double TupleRestraint<Score>
::unprotected_incremental_evaluate(DerivativeAccumulator *accum) const
{
  if (ss_->get_is_changed(v_)) {
    score_+=ss_->Score::evaluate_change(v_, accum);
  }
  return score_;
}

template <class Score>
ParticlesTemp TupleRestraint<Score>::get_input_particles() const
{
  return IMP::internal::get_input_particles(ss_.get(), v_);
}

template <class Score>
ContainersTemp TupleRestraint<Score>::get_input_containers() const
{
  return IMP::internal::get_input_containers(ss_.get(), v_);
}

template <class Score>
void TupleRestraint<Score>::do_show(std::ostream& out) const
{
  out << "score " << *ss_ << std::endl;
  out << "data " << IMP::internal::streamable(v_).get_name() << std::endl;
}
#ifndef IMP_DOXYGEN
template <class Score>
Restraint* create_restraint(Pointer<Score> s,
                            const typename Score::Argument &t,
                            std::string name= std::string()) {
  return create_restraint<Score>(s, t, name);
}
#endif











template <class Before, class After>
TupleConstraint<Before, After>::TupleConstraint(Before *before,
                                                After *after,
                               const typename Before::Argument& vt,
                                                std::string name):
  Constraint(name), v_(vt){
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
  f_->Before::apply(v_);
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
  af_->After::apply(v_, *da);
  IMP_LOG(TERSE, "End GroupnamesConstraint::after_evaluate" << std::endl);
}

template <class Before, class After>
ContainersTemp TupleConstraint<Before, After>::get_input_containers() const {
  return ContainersTemp();
}

template <class Before, class After>
ContainersTemp TupleConstraint<Before, After>::get_output_containers() const {
  return ContainersTemp();
}

template <class Before, class After>
ParticlesTemp TupleConstraint<Before, After>::get_input_particles() const {
  ParticlesTemp ret;
  if (f_) {
    ret= IMP::internal::get_input_particles(f_.get(), v_);
    ParticlesTemp o= IMP::internal::get_output_particles(f_.get(), v_);
    ret.insert(ret.end(), o.begin(), o.end());
    IMP_IF_CHECK(USAGE) {
      if (af_) {
        ParticlesTemp oret= IMP::internal::get_input_particles(af_.get(), v_);
        std::sort(ret.begin(), ret.end());
        std::sort(oret.begin(), oret.end());
        ParticlesTemp t;
        std::set_union(ret.begin(), ret.end(), oret.begin(), oret.end(),
                       std::back_inserter(t));
        IMP_USAGE_CHECK(t.size() == ret.size(),
                        "The particles written by "
                        << " the after modifier in " << get_name() << " must "
                        << "be a subset of those read by the before "
                        << "modifier. Before: " << Particles(ret)
                        << " and after " << Particles(oret));
      }
    }
  } else {
    ret=IMP::internal::get_output_particles(af_.get(), v_);
  }
  return ret;
}

template <class Before, class After>
ParticlesTemp TupleConstraint<Before, After>::get_output_particles() const {
  ParticlesTemp ret;
  if (f_) {
    ret= IMP::internal::get_output_particles(f_.get(), v_);
    IMP_IF_CHECK(USAGE) {
      if (af_) {
        ParticlesTemp oret= IMP::internal::get_input_particles(af_.get(), v_);
        ParticlesTemp iret=IMP::internal::get_input_particles(f_.get(), v_);
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
    ret= IMP::internal::get_input_particles(af_.get(), v_);
  }
  return ret;
}

template <class Before, class After>
void TupleConstraint<Before, After>::do_show(std::ostream &out) const {
  out << "on " << IMP::internal::streamable(v_).get_name() << std::endl;
  if (f_) out << "before " << *f_ << std::endl;
  if (af_) out << "after " << *af_ << std::endl;
}

#ifndef IMP_DOXYGEN
template <class Before, class After>
Constraint* create_constraint(Pointer<Before> b, Pointer<After> a,
                              const typename Before::Argument &t,
                              std::string name=std::string()) {
  return create_constraint<Before, After>(b, a, t, name);
}
template <class Before, class After>
Constraint* create_constraint(Before* b, Pointer<After> a,
                              const typename Before::Argument &t,
                              std::string name=std::string()) {
  return create_constraint<Before, After>(b, a, t, name);
}
template <class Before, class After>
Constraint* create_constraint(Pointer<Before> b, After* a,
                              const typename Before::Argument &t,
                              std::string name=std::string()) {
  return create_constraint<Before, After>(b, a, t, name);
}
#endif

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_GENERIC_IMPL_H */
