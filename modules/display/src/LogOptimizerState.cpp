/**
 *  \file LogOptimizerState.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/LogOptimizerState.h"


IMPDISPLAY_BEGIN_NAMESPACE

LogOptimizerState::LogOptimizerState(Writer *w, std::string name ):
  writer_(w), step_(0), skip_steps_(0), name_template_(name){
}

LogOptimizerState::~LogOptimizerState(){}

void LogOptimizerState::show(std::ostream &out) const {
  out << "LogOptimizerState" << std::endl;
}

void LogOptimizerState::update() {
  ++step_;
  if (step_%(skip_steps_+1)==0) {
    unsigned int n= step_;
    n/= (skip_steps_+1);
    char buf[1000];
    sprintf(buf, name_template_.c_str(), n);
    IMP_LOG(TERSE, "Writing file " << buf << std::endl);
    writer_->set_file_name(buf);
    IMP_LOG(VERBOSE, "Writing extractors"<< std::endl);
    for (unsigned int i=0; i< edata_.size(); ++i) {
      for (SingletonContainer::ParticleIterator
             pit =edata_[i].second->particles_begin();
           pit!= edata_[i].second->particles_end(); ++pit) {
        Geometries gs=edata_[i].first->get_geometry(*pit);
        writer_->add_geometry(gs);
        // since swig sucks
        for (unsigned int i=0; i< gs.size(); ++i) {
          Pointer<Geometry>(gs[i]);
        }
      }
    }
    IMP_LOG(VERBOSE, "Writing geometries"<< std::endl);
    for (unsigned int i=0; i < gdata_.size(); ++i) {
      writer_->add_geometry(gdata_[i]);
    }
    writer_->set_file_name("");
  }
}

IMPDISPLAY_END_NAMESPACE
