/**
 *  \file ConfiguratioNSet.cpp
 *  \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/ConfigurationSet.h"
#include "IMP/internal/utility.h"
#include "IMP/io.h"

IMP_BEGIN_NAMESPACE

ConfigurationSet::ConfigurationSet(Model *m,
                                   std::string nm):
  Object(nm),
  model_(m), base_(new Configuration(m)){
}


void ConfigurationSet::save_configuration() {
  IMP_OBJECT_LOG;
  set_was_used(true);
  IMP_LOG(TERSE, "Adding configuration to set " << get_name() << std::endl);
  configurations_.push_back(new Configuration(model_, base_));
}

void ConfigurationSet::remove_configuration(unsigned int i) {
  IMP_USAGE_CHECK(i < get_number_of_configurations(),
                  "Out of range configuration: " << i);
  configurations_.erase(configurations_.begin()+i);
}

unsigned int ConfigurationSet::get_number_of_configurations() const {
  return configurations_.size();
}

void ConfigurationSet::load_configuration(int i) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(this);
  IMP_USAGE_CHECK(i < static_cast<int>(get_number_of_configurations())
                  && i >= -1,
                  "Invalid configuration requested.");
  if (i==-1) {
    base_->load_configuration();
  } else {
    configurations_[i]->load_configuration();
  }
}

IMP_END_NAMESPACE
