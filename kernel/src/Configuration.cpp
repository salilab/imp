/**
 *  \file Configuration.cpp
 *  \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Configuration.h"
#include "IMP/internal/utility.h"
#include "IMP/dependency_graph.h"

IMP_BEGIN_NAMESPACE



Configuration::Configuration(Model *m, std::string name): Object(name),
  model_(m){
  for (Model::ParticleIterator it= model_->particles_begin();
       it != model_->particles_end(); ++it) {
    PP pp(*it);
    base_[pp]= internal::ParticleData(*it);
  }
}




void Configuration::load_configuration() const {
  IMP_OBJECT_LOG;
  Particles to_remove;
  // do not invalidate my iterator
  for (Model::ParticleIterator it= model_->particles_begin();
       it != model_->particles_end(); ++it) {
    PP pp(*it);
    if (base_.find(pp) == base_.end()) {
      to_remove.push_back(*it);
    }
  }
  for (unsigned int i=0; i< to_remove.size(); ++i) {
    IMP_LOG(VERBOSE, "Removing particle " << to_remove[i]->get_name()
            << " from model." << std::endl);
    model_->remove_particle(to_remove[i]);
  }
  internal::Set<Particle*> active(model_->particles_begin(),
                             model_->particles_end());
  for (DataMap::const_iterator it= base_.begin(); it != base_.end(); ++it) {
    PP pp(it->first);
    if (active.find(pp) == active.end()) {
      model_->restore_particle(it->first);
    }
    it->second.apply(it->first);
  }
}


void Configuration::do_show(std::ostream &out) const {
  out <<  "configuration" << std::endl;
}

IMP_END_NAMESPACE
