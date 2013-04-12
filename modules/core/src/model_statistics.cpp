/**
 *  \file AngleRestraint.cpp \brief Angle restraint between three particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/model_statistics.h>
#include <IMP/base/file.h>
IMPCORE_BEGIN_NAMESPACE
WriteRestraintScoresOptimizerState
::WriteRestraintScoresOptimizerState(const Restraints &rs,
                                     base::TextOutput out):
    PeriodicOptimizerState("WriteRestraintScoresOptimizerState%1%"),
    rs_(rs),
    out_(out){
  for (unsigned int i=0; i< rs_.size(); ++i) {
    if (i!= 0) out_.get_stream() << ", ";
    out_.get_stream() << rs_[i]->get_name();
  }
  out_.get_stream() << std::endl;
  if (rs.size() >0) {
    rs[0]->get_model()->set_gather_statistics(true);
  }
}




void WriteRestraintScoresOptimizerState::do_update(unsigned int) {
  for (unsigned int i=0;i< rs_.size(); ++i) {
    if (i!= 0) out_.get_stream() << ", ";
    out_ .get_stream()<< rs_[i]->get_last_score();
  }
  out_.get_stream() << std::endl;
}

IMPCORE_END_NAMESPACE
