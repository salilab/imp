/**
 *  \file RestraintSet.cpp   \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */


#include <IMP/RestraintSet.h>
#include <IMP/Model.h>
#include <IMP/log.h>
#include <IMP/internal/utility.h>
#include <memory>
#include <utility>

IMP_BEGIN_NAMESPACE


RestraintSet::RestraintSet(double weight,
                           const std::string& name)
  : Restraint(name), weight_(weight)
{
}

RestraintSet::RestraintSet(const std::string& name)
  : Restraint(name), weight_(1.0)
{
}




IMP_LIST_IMPL(RestraintSet, Restraint, restraint, Restraint*,
              Restraints,
              {
                if (get_is_part_of_model()) {
                  obj->set_model(get_model());
                  get_model()->reset_dependencies();
                }
                obj->set_was_used(true);
                IMP_USAGE_CHECK(obj != this,
                                "Cannot add a restraint set to itself");
              },if (get_is_part_of_model()) {
                  get_model()->reset_dependencies();
              },{
                if (container) obj->get_model()->reset_dependencies();
                obj->set_model(NULL);
              });



void RestraintSet::set_weight(double w) {
  weight_=w;
  if (get_is_part_of_model()) {
    get_model()->reset_dependencies();
  }
}

double
RestraintSet::evaluate(bool deriv) const
{
  RestraintsTemp restraints;
  std::vector<double> weights;
  boost::tie(restraints, weights)=
    get_restraints_and_weights(this);
  return get_model()->evaluate(restraints, weights, deriv);
}

double
RestraintSet::unprotected_evaluate(DerivativeAccumulator *) const
{
  IMP_FAILURE("RestraintSets are special cased in the Model");
}

void RestraintSet::set_model(Model *m) {
  Restraint::set_model(m);
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    (*it)->set_model(m);
  }
}

ParticlesTemp RestraintSet::get_input_particles() const
{
  IMP_FAILURE("RestraintSets are special cased in the Model");
}

ContainersTemp RestraintSet::get_input_containers() const {
  IMP_FAILURE("RestraintSets are special cased in the Model");
}

void RestraintSet::do_show(std::ostream& out) const
{
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    (*it)->show(out);
  }
  out << "... end restraint set " << get_name() << std::endl;
}


RestraintsAndWeights get_restraints_and_weights(const RestraintsTemp &rs,
                                    double initial_weight) {
  return get_restraints_and_weights(rs.begin(), rs.end(), initial_weight);
}


RestraintsAndWeights get_restraints_and_weights(const RestraintSet *rs) {
  return get_restraints_and_weights(rs->restraints_begin(),
                                    rs->restraints_end(),
                                    rs->get_weight());
}

RestraintsTemp get_restraints(const RestraintsTemp &rs) {
  return get_restraints(rs.begin(), rs.end());
}


RestraintsTemp get_restraints(const RestraintSet *rs) {
  return get_restraints(rs->restraints_begin(), rs->restraints_end());
}

namespace {
  unsigned int num_children(Restraint*r) {
    RestraintSet *rs= dynamic_cast<RestraintSet*>(r);
    if (rs) return rs->get_number_of_restraints();
    else return 0;
  }
}

IMPEXPORT void show_restraint_hierarchy(RestraintSet *rs, std::ostream &out) {
  IMP_PRINT_TREE(out, Restraint*, rs, num_children(n),
                 dynamic_cast<RestraintSet*>(n)->get_restraint,
                 out << n->get_name());
}

IMP_END_NAMESPACE
