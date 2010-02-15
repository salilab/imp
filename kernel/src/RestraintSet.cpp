/**
 *  \file RestraintSet.cpp   \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */


#include <IMP/RestraintSet.h>

#include <IMP/log.h>

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
                obj->set_was_owned(true);
              },,{
                if (get_is_part_of_model()) {
                  get_model()->reset_dependencies();
                }
                obj->set_model(NULL);
              });



void RestraintSet::set_weight(double w) {
  weight_=w;
  get_model()->reset_dependencies();
}

double
RestraintSet::unprotected_evaluate(DerivativeAccumulator *accum) const
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

ParticlesList RestraintSet::get_interacting_particles() const
{
  IMP_FAILURE("RestraintSets are special cased in the Model");
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

IMP_END_NAMESPACE
