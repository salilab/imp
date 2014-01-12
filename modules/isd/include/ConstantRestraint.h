/**
 *  \file IMP/isd/ConstantRestraint.h
 *  \brief kernel::Restraint and ScoreState for Constant
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_CONSTANT_RESTRAINT_H
#define IMPISD_CONSTANT_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/macros.h>
#include <boost/scoped_ptr.hpp>
#include <IMP/kernel/Restraint.h>
#include <IMP/base/Pointer.h>

#include <IMP/ScoreState.h>

IMPISD_BEGIN_NAMESPACE

class ConstantScoreState;

class IMPISDEXPORT ConstantRestraint : public kernel::Restraint {
 private:
  IMP::base::PointerMember<ConstantScoreState> ss_;

 public:
  ConstantRestraint(kernel::Model *m);

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const
      IMP_OVERRIDE;

  void update() const { std::cout << "updated internal state" << std::endl; }

  IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(ConstantRestraint);

  // to allow the scorestate to get the restraint's objects
  friend class ConstantScoreState;
};

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
class IMPISDEXPORT ConstantScoreState : public ScoreState {
 private:
  IMP::base::WeakPointer<ConstantRestraint> cr_;

 private:
  ConstantScoreState(ConstantRestraint *cr)
      : ScoreState(cr->get_model(), "ConstantScoreState%1%"), cr_(cr) {}

 public:
  // only the cr can create this and add it to the model
  friend class ConstantRestraint;
  virtual void do_before_evaluate() IMP_OVERRIDE;
  virtual void do_after_evaluate(DerivativeAccumulator *da) IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(ConstantScoreState);
};

void ConstantScoreState::do_before_evaluate() {
  std::cout << "css: do_before_evaluate" << std::endl;
  cr_->update();
}

void ConstantScoreState::do_after_evaluate(DerivativeAccumulator *) {
  std::cout << "css: do_after_evaluate" << std::endl;
}

ModelObjectsTemp ConstantScoreState::do_get_inputs() const {
  return kernel::ModelObjectsTemp();
}

ModelObjectsTemp ConstantScoreState::do_get_outputs() const {
  return kernel::ModelObjectsTemp();
}

#endif

ConstantRestraint::ConstantRestraint(kernel::Model *m)
    : Restraint(m, "ConstantRestraint %1%") {
  ss_ = new ConstantScoreState(this);
}

ModelObjectsTemp ConstantRestraint::do_get_inputs() const {
  kernel::ModelObjectsTemp ret;
  if (ss_) ret.push_back(ss_);
  return ret;
}

double ConstantRestraint::unprotected_evaluate(DerivativeAccumulator *accum)
    const {
  std::cout << "cr: evaluate" << std::endl;
  return 1.0;
}

IMPISD_END_NAMESPACE

#endif /* IMPISD_CONSTANT_RESTRAINT_H */
