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

bool Sampler::get_is_good_configuration() {
  if (max_score_ != std::numeric_limits<double>::max()) {
    double e= model_->evaluate(false);
    if (e > max_score_) return false;
  }
  for (Maxes::const_iterator it= max_scores_.begin();
       it != max_scores_.end(); ++it) {
    double e= model_->evaluate(RestraintsTemp(1, it->first), false);
    if (e > it->second) return false;
  }
  return true;
}

Sampler::~Sampler(){}

IMP_END_NAMESPACE
