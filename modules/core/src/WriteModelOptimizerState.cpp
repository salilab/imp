/**
 *  \file LogOptimizerState.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/WriteModelOptimizerState.h"


IMPCORE_BEGIN_NAMESPACE

WriteModelOptimizerState::WriteModelOptimizerState(Model *m, std::string name ):
  model_(m), name_template_(name){
  set_name(std::string("WriteModel to ") + name);
}

void WriteModelOptimizerState::do_show(std::ostream &out) const {
  out << "name " << name_template_ << std::endl;
}

void WriteModelOptimizerState::do_update(unsigned int n) {
  IMP_OBJECT_LOG;
  char buf[1000];
  sprintf(buf, name_template_.c_str(), n);
  IMP_LOG(TERSE, "Writing file " << buf << std::endl);
  write_model(model_, buf);
}

IMPCORE_END_NAMESPACE
