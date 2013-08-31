/**
 *   Copyright 2007-2013 IMP Inventors. All rights reserved
 */
#include <IMP/Model.h>
#include <IMP/kernel/Particle.h>
#include <IMP/kernel/Restraint.h>
#include <IMP/restraint_macros.h>
#include <IMP/base/threads.h>
#include <IMP/base/flags.h>

#if IMP_HAS_CHECKS >= IMP_INTERNAL

namespace {

std::string get_module_version() { return std::string(); }

std::string get_module_name() { return std::string(); }

class TouchyRestraint : public IMP::Restraint {
  IMP::kernel::Particle *p_;
  IMP::FloatKey fk_;

 public:
  TouchyRestraint(IMP::kernel::Particle *p, IMP::FloatKey fk)
      : IMP::Restraint(p->get_model(), "Touchy"), p_(p), fk_(fk) {}
  virtual double unprotected_evaluate(
      IMP::kernel::DerivativeAccumulator *accum) const IMP_OVERRIDE;
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(TouchyRestraint);
};

double TouchyRestraint::unprotected_evaluate(
    IMP::DerivativeAccumulator *) const {
  return p_->get_value(fk_);
}

IMP::ModelObjectsTemp TouchyRestraint::do_get_inputs() const {
  return IMP::ModelObjectsTemp();
}
}
#endif

int main(int argc, char *argv[]) {
  IMP::base::setup_from_argv(argc, argv, "Testing protection of particles");
// no checks in fast mode
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  IMP_NEW(IMP::Model, m, ());
  IMP_NEW(IMP::kernel::Particle, p, (m));
  IMP::base::SetNumberOfThreads no(1);
  IMP_NEW(TouchyRestraint, r, (p, IMP::FloatKey(0)));
  try {
    r->evaluate(false);
    // there had better be an exception
    return 1;
    // the exception gets translated into a normal IMP exception
  }
  catch (const std::runtime_error & e) {
    std::cerr << e.what() << std::endl;
  }
#endif
  return 0;
}
