/**
 *  \file MinimumRestraint.cpp \brief Distance restraint between two particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/MinimumRestraint.h>
#include <IMP/algebra/internal/MinimalSet.h>
#include <IMP/Model.h>

IMPCORE_BEGIN_NAMESPACE

MinimumRestraint::MinimumRestraint(unsigned int num, const Restraints &rs,
                                   std::string name)
    : Restraint(rs[0]->get_model(), name), k_(num) {
  set_restraints(rs);
}

IMP_LIST_IMPL(MinimumRestraint, Restraint, restraint, Restraint *, Restraints);

double MinimumRestraint::unprotected_evaluate(DerivativeAccumulator *da) const {
  algebra::internal::MinimalSet<double, Restraint *> ms(k_);
  for (RestraintConstIterator it = restraints_begin(); it != restraints_end();
       ++it) {
    ms.insert((*it)->unprotected_evaluate(nullptr), *it);
  }
  if (!da) {
    double sum = 0;
    for (unsigned int i = 0; i < ms.size(); ++i) {
      sum += ms[i].first;
    }
    return sum;
  } else {
    double sum = 0;
    for (unsigned int i = 0; i < ms.size(); ++i) {
      sum += ms[i].second->unprotected_evaluate(da);
    }
    return sum;
  }
}

void MinimumRestraint::set_model(Model *m) {
  Restraint::set_model(m);
  for (RestraintConstIterator it = restraints_begin(); it != restraints_end();
       ++it) {
    (*it)->set_model(m);
  }
}

ModelObjectsTemp MinimumRestraint::do_get_inputs() const {
  ModelObjectsTemp ret;
  for (unsigned int i = 0; i < get_number_of_restraints(); ++i) {
    ret += get_restraint(i)->get_inputs();
  }
  return ret;
}

void MinimumRestraint::on_add(Restraint* r) const {
  r->set_model(get_model());
}

IMPCORE_END_NAMESPACE
