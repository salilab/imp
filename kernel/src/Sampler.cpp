/**
 *  \file Sampler.cpp
 *  \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Sampler.h"
#include <limits>

IMP_BEGIN_NAMESPACE


Sampler::Sampler(Model *m,
                 std::string nm): Object(nm),
                                  model_(m){
}

ConfigurationSet *Sampler::get_sample() const {
  IMP_OBJECT_LOG;
  unsigned int num_opt=0;
  set_was_used(true);
  for (Model::ParticleConstIterator pit= model_->particles_begin();
       pit != model_->particles_end(); ++pit) {
    unsigned int d=std::distance((*pit)->optimized_keys_begin(),
                  (*pit)->optimized_keys_end());
    if (d > 0) ++num_opt;
  }
  //IMP_LOG(TERSE, "Sampling " << num_opt << " particles."<<std::endl);
  /*if (num_opt == 0) {
    IMP_WARN("There are no particles to optimize."<<std::endl);
    return NULL;
    }*/
  return do_sample();
}



Sampler::~Sampler(){}

IMP_END_NAMESPACE
