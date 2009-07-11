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
  IMP_LOG(TERSE, "Writing log file " << name << std::endl);
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
  IMP_OBJECT_LOG;
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





DisplayModelOnFailure::DisplayModelOnFailure(LogOptimizerState *s,
                                       std::string f): s_(s),
                                                       file_name_(f){}

void DisplayModelOnFailure::handle_failure() {
  s_->write(file_name_);
}

void DisplayModelOnFailure::show(std::ostream &out) const {
  out << "DisplayModelOnFailure" << std::endl;
}

IMPDISPLAY_END_NAMESPACE
