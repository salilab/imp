/**
 *  \file Configuration.cpp
 *  \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Configuration.h"
#include "IMP/internal/utility.h"
#include "IMP/dependency_graph.h"
#include "IMP/compatibility/set.h"

IMP_BEGIN_NAMESPACE



Configuration::Configuration(Model *m, std::string name): Object(name),
  model_(m){
  floats_= *m;
  strings_=*m;
  ints_=*m;
  objects_=*m;
  ints_lists_=*m;
  objects_lists_=*m;
  particles_=*m;
  particles_lists_=*m;
}




void Configuration::load_configuration() const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  static_cast<FloatAttributeTable&>(*model_)= floats_;
  static_cast<StringAttributeTable&>(*model_)= strings_;
  static_cast<IntAttributeTable&>(*model_)= objects_;
  static_cast<ObjectAttributeTable&>(*model_)= ints_lists_;
  static_cast<IntsAttributeTable&>(*model_)= objects_lists_;
  static_cast<ObjectsAttributeTable&>(*model_)= particles_;
  static_cast<ParticleAttributeTable&>(*model_)= particles_lists_;
  static_cast<ParticlesAttributeTable&>(*model_)= ints_;
}


void Configuration::do_show(std::ostream &out) const {
  out <<  "configuration" << std::endl;
}

IMP_END_NAMESPACE
