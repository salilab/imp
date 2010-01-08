/**
 *  \file Sampler.cpp
 *  \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Sampler.h"
#include <limits>

IMP_BEGIN_NAMESPACE


Sampler::Sampler(Model *m,
                 std::string nm): Object(nm),
                                  model_(m){
  max_score_= std::numeric_limits<double>::max();
}

bool Sampler::get_is_good_configuration() const {
  bool ret=true;
  if (max_score_ != std::numeric_limits<double>::max()) {
    double e= get_model()->evaluate(false);
    if (e > max_score_) {
      ret=false;
    }
  }
  if (ret) {
    for (Maxes::const_iterator it= max_scores_.begin();
         it != max_scores_.end(); ++it) {
      double e= get_model()->evaluate(RestraintsTemp(1, it->first), false);
      if (e > it->second) {
        ret=false;
        break;
      }
    }
  }
  if (ret) {
    IMP_LOG(TERSE, "Configuration accepted."<< std::endl);
  } else {
    IMP_LOG(TERSE, "Configuration rejected."<< std::endl);
  }
  return ret;
}

Sampler::~Sampler(){}

IMP_END_NAMESPACE
