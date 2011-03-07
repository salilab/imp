/**
 *  \file MinimumRestraint.cpp \brief Distance restraint between two particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/MinimumRestraint.h>
#include <IMP/algebra/internal/MinimalSet.h>
#include <IMP/Model.h>

IMPCORE_BEGIN_NAMESPACE

MinimumRestraint::MinimumRestraint(unsigned int num,
                                   const Restraints& rs,
                                   std::string name) :
  Restraint(name), k_(num)
{
  set_restraints(rs);
}


IMP_LIST_IMPL(MinimumRestraint, Restraint, restraint, Restraint*, Restraints,
              {
                if (get_is_part_of_model()) {
                  get_model()->reset_dependencies();
                  obj->set_model(get_model());
                }
              }
              ,{
                if (get_is_part_of_model()) {
                  get_model()->reset_dependencies();
                }
              }, {
                if (container) obj->get_model()->reset_dependencies();
                obj->set_model(NULL);
              });


double
MinimumRestraint::unprotected_evaluate(DerivativeAccumulator *da) const
{
  algebra::internal::MinimalSet<double, Restraint*> ms(k_);
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    ms.insert((*it)->unprotected_evaluate(NULL), *it);
  }
  if (!da) {
    double sum=0;
    for (unsigned int i=0; i< ms.size(); ++i) {
      sum+= ms[i].first;
    }
    return sum;
  } else {
    double sum=0;
    for (unsigned int i=0; i< ms.size(); ++i) {
      sum+= ms[i].second->unprotected_evaluate(da);
    }
    return sum;
  }
}

void MinimumRestraint::set_model(Model *m) {
  Restraint::set_model(m);
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    (*it)->set_model(m);
  }
}

ParticlesTemp MinimumRestraint::get_input_particles() const
{
  ParticlesTemp all;
  for (unsigned int i=0; i< get_number_of_restraints(); ++i) {
    ParticlesTemp cur= get_restraint(i)->get_input_particles();
    all.insert(all.end(), cur.begin(), cur.end());
  }
  return all;
}

ContainersTemp MinimumRestraint::get_input_containers() const {
  ContainersTemp all;
  for (unsigned int i=0; i< get_number_of_restraints(); ++i) {
    ContainersTemp cur= get_restraint(i)->get_input_containers();
    all.insert(all.end(), cur.begin(), cur.end());
  }
  return all;
}

void MinimumRestraint::do_show(std::ostream& out) const
{
  out << "min k: " << k_ << std::endl;
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    (*it)->show(out);
  }
}


IMPCORE_END_NAMESPACE
