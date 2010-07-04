/**
 *  \file LogOptimizerState.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/WriteModelOptimizerState.h"
#include <boost/format.hpp>
#include <sstream>
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
  std::ostringstream oss;
  if (p_) {
    oss << boost::format(name_template_)% p_->get_value(k_);
  } else {
    oss << boost::format(name_template_)% n;
  }
  IMP_LOG(TERSE, "Writing file " << oss.str() << std::endl);
  write_model(model_, oss.str());
}

IMPCORE_END_NAMESPACE
