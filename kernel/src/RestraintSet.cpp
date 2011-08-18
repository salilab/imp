/**
 *  \file RestraintSet.cpp   \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */


#include <IMP/RestraintSet.h>
#include <IMP/Model.h>
#include <IMP/log.h>
#include <IMP/internal/utility.h>
#include <boost/tuple/tuple.hpp>
#include <memory>
#include <utility>
#include <numeric>

IMP_BEGIN_NAMESPACE


RestraintSet::RestraintSet(double weight,
                           const std::string& name)
  : Restraint(name)
{
  set_weight(weight);
}

RestraintSet::RestraintSet(const std::string& name)
  : Restraint(name)
{
}




IMP_LIST_IMPL(RestraintSet, Restraint, restraint, Restraint*,
              Restraints);


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
  IMP_CHECK_OBJECT(this);
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    (*it)->show(out);
  }
  out << "... end restraint set " << get_name() << std::endl;
}


void RestraintSet::on_add(Restraint*obj) {
if (get_is_part_of_model()) {
                      obj->set_model(get_model());
                      get_model()->reset_dependencies();
                    }
                    obj->set_was_used(true);
                    IMP_USAGE_CHECK(obj != this,
                                    "Cannot add a restraint set to itself");
}
void RestraintSet::on_change() {
  if (get_is_part_of_model()) {
    get_model()->reset_dependencies();
  }
}
void RestraintSet::on_remove(RestraintSet *container, Restraint* obj) {
  if (container) obj->get_model()->reset_dependencies();
  obj->set_model(NULL);
}


RestraintsAndWeights get_restraints_and_weights(const RestraintsTemp &rs,
                                    double initial_weight) {
  return get_restraints_and_weights(rs.begin(), rs.end(), initial_weight);
}


RestraintsTemp get_restraints(const RestraintsTemp &rs) {
  return get_restraints(rs.begin(), rs.end());
}


namespace {
  unsigned int num_children(Restraint*r) {
    RestraintSet *rs= dynamic_cast<RestraintSet*>(r);
    if (rs) return rs->get_number_of_restraints();
    else return 0;
  }
}

void show_restraint_hierarchy(RestraintSet *rs, std::ostream &out) {
  IMP_PRINT_TREE(out, Restraint*, rs, num_children(n),
                 dynamic_cast<RestraintSet*>(n)->get_restraint,
                 out << n->get_name());
}



Restraints create_decomposition(const RestraintsTemp &rs) {
  IMP_FUNCTION_LOG;
  if (rs.empty()) return Restraints();
  Restraints ret;
  RestraintsTemp all= get_restraints(rs);
  for (unsigned int i=0; i< all.size(); ++i) {
    Restraints cur= all[i]->create_decomposition();
    ret.insert(ret.end(), cur.begin(), cur.end());
  }
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    RestraintsTemp frs= get_restraints(rs);
    RestraintsTemp fret= get_restraints(RestraintsTemp(ret.begin(),
                                                       ret.end()));
    Floats efrs= rs[0]->get_model()->evaluate(frs, false);
    Floats efret= rs[0]->get_model()->evaluate(fret, false);
    double s0= std::accumulate(efrs.begin(), efrs.end(), 0);
    double s1= std::accumulate(efret.begin(), efret.end(), 0);
    IMP_INTERNAL_CHECK(std::abs(s0-s1) < .1*std::abs(s0+s1)+.1,
                       "The before and after scores don't agree: "
                       << s0 << " vs " << s1);
  }
  return ret;
}

IMP_END_NAMESPACE
