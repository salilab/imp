/**
 *  \file LogOptimizerState.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/LogOptimizerState.h"


IMPDISPLAY_BEGIN_NAMESPACE

LogOptimizerState::LogOptimizerState(Writer *w, std::string name ):
  writer_(w), step_(0), skip_steps_(0), name_template_(name){
}

void LogOptimizerState::show(std::ostream &out) const {
  out << "LogOptimizerState" << std::endl;
}


void LogOptimizerState::write(std::string name) const {
  writer_->set_file_name(name);
  IMP_LOG(VERBOSE, "Writing extractors"<< std::endl);
  for (unsigned int i=0; i< edata_.size(); ++i) {
    Geometries gs=edata_[i]->get_geometry();
    writer_->add_geometry(gs);
  }
  IMP_LOG(VERBOSE, "Writing geometries"<< std::endl);
  for (unsigned int i=0; i < gdata_.size(); ++i) {
    writer_->add_geometry(gdata_[i]);
  }
  writer_->set_file_name("");
}

void LogOptimizerState::update() {
  ++step_;
  if (step_%(skip_steps_+1)==0) {
    unsigned int n= step_;
    n/= (skip_steps_+1);
    --n;
    char buf[1000];
    sprintf(buf, name_template_.c_str(), n);
    IMP_LOG(TERSE, "Writing file " << buf << std::endl);
    write(buf);
  }
}

namespace {
  Pointer<LogOptimizerState> error_log;
  std::string error_name;
  int error_index;
  bool throw_more;

  bool log_failure_function(std::string message) {
    std::string fname;
    if (error_name.find("%d") != std::string::npos) {
      char buf[1000];
      sprintf(buf, error_name.c_str(), error_index);
      ++error_index;
      fname= buf;
    } else {
      fname= error_name;
    }
    error_log->write(error_name);
    return throw_more;
  }
}

void set_failure_display_log(LogOptimizerState *log,
                             std::string file_name) {
  error_name=file_name;
  error_log=log;
  error_index=0;
  throw_more= true;
  set_failure_function(log_failure_function);
}



void LogOptimizerState::add_geometry(CompoundGeometry* g) {
  edata_.push_back(Pointer<CompoundGeometry>(g));
}

void LogOptimizerState::add_geometry(Geometry* g) {
  gdata_.push_back(Pointer<Geometry>(g));
}

void LogOptimizerState::add_geometry(const CompoundGeometries& g) {
  for (unsigned int i=0; i< g.size(); ++i) {
    add_geometry(g);
  }
}

void LogOptimizerState::add_geometry(const Geometries& g) {
  for (unsigned int i=0; i< g.size(); ++i) {
    add_geometry(g);
  }
}

IMPDISPLAY_END_NAMESPACE
