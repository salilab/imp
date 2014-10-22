/**
 *  \file isd/LogWrapper.cpp
 *  \brief Calculate the -Log of a list of restraints.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/LogWrapper.h>
#include <IMP/kernel/internal/scoring_functions.h>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

LogWrapper::LogWrapper(kernel::Model *m, double weight,
                       const std::string &name)
  : Restraint(m, name) {
  set_weight(weight);
}

LogWrapper::LogWrapper(kernel::Model *m, const std::string &name)
  : Restraint(m, name) {
  set_weight(1.0);
}

LogWrapper::LogWrapper(const RestraintsTemp &rs,double weight,
                       const std::string &name)
  : Restraint(kernel::internal::get_model(rs),name){
  set_weight(weight);
  set_restraints(rs);
}

IMP_LIST_IMPL(LogWrapper, Restraint, restraint, Restraint *, Restraints);


double LogWrapper::unprotected_evaluate(DerivativeAccumulator* accum) const {
    double prob = 1;
    double score = 0;

    for (unsigned int i = 0; i <get_number_of_restraints(); ++i) {
      prob *= get_restraint(i)->unprotected_evaluate(accum);
      if (prob<=std::numeric_limits<double>::min()*1000000.0){
        score=score-std::log(prob);
        prob=1.0;
      }
    }

    score=score-std::log(prob);
    if (accum) {
      // derivatives should be accurately handled in the restraints
    }
    return score;
}

void LogWrapper::show_it(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  for (RestraintConstIterator it = restraints_begin(); it != restraints_end();
       ++it) {
    (*it)->show(out);
  }
  out << "... end LogWrapper " << get_name() << std::endl;
}

void LogWrapper::on_add(Restraint *obj) {
  set_has_dependencies(false);
  obj->set_was_used(true);
  IMP_USAGE_CHECK(obj != this, "Cannot add a LogWrapper to itself");
}
void LogWrapper::on_change() { set_has_dependencies(false); }

ModelObjectsTemp LogWrapper::do_get_inputs() const {
  return ModelObjectsTemp(restraints_begin(), restraints_end());
}
void LogWrapper::on_remove(LogWrapper *container, Restraint *) {
  if (container && container->get_model()) {
    container->set_has_dependencies(false);
  }
}

IMPISD_END_NAMESPACE
