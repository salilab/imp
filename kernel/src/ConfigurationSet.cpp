/**
 *  \file ConfiguratioNSet.cpp
 *  \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include "IMP/ConfigurationSet.h"
#include "IMP/internal/utility.h"

IMP_BEGIN_NAMESPACE
namespace {
  unsigned int restraint_index=0;
}


ConfigurationSet::ConfigurationSet(Model *m,
                                   std::string nm):
  Object(internal::make_object_name(nm, restraint_index++)),
  model_(m){
  for (Model::ParticleIterator it= model_->particles_begin();
       it != model_->particles_end(); ++it) {
    PP pp(*it);
    base_[pp]= internal::ParticleData(*it);
  }
  IMP_IF_LOG(TERSE) {
    IMP_LOG(TERSE, "Base configuration is:\n");
    for (Model::ParticleIterator it= model_->particles_begin();
       it != model_->particles_end(); ++it) {
      IMP_LOG(TERSE, (*it)->get_name() << ":\n");
      PP pp(*it);
      IMP_LOG_WRITE(TERSE, base_[pp].show(IMP_STREAM));
    }
  }
}


void ConfigurationSet::save_configuration() {
  IMP_OBJECT_LOG;
  set_was_owned(true);
  IMP_LOG(TERSE, "Adding configuration to set " << get_name() << std::endl);
  configurations_.push_back(Diff());
  for (Model::ParticleIterator it= model_->particles_begin();
       it != model_->particles_end(); ++it) {
    PP pp(*it);
    if (base_.find(pp) != base_.end()) {
      configurations_.back().diffs_[pp]
        = internal::ParticleDiff(base_[pp], pp);
    } else {
      configurations_.back().added_[pp]= internal::ParticleData(pp);
    }
  }
  for (DataMap::iterator it= base_.begin(); it != base_.end(); ++it) {
    PP pp(it->first);
    if (configurations_.back().diffs_.find(pp)
        == configurations_.back().diffs_.end()) {
      // removed particle
      configurations_.back().removed_.insert(it->first);
    }
  }
}

unsigned int ConfigurationSet::get_number_of_configurations() const {
  return configurations_.size();
}

void ConfigurationSet::set_base() {
  IMP_OBJECT_LOG;
  for (Model::ParticleIterator it= model_->particles_begin();
       it != model_->particles_end(); ++it) {
    PP pp(*it);
    if (base_.find(pp) == base_.end()) {
      model_->remove_particle(*it);
    }
  }
  std::set<Particle*> active(model_->particles_begin(),
                             model_->particles_end());
  for (DataMap::iterator it= base_.begin(); it != base_.end(); ++it) {
    PP pp(it->first);
    if (active.find(pp) == active.end()) {
      model_->restore_particle(it->first);
    }
    it->second.apply(it->first);
  }
}

void ConfigurationSet::set_configuration(int i) {
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(i < static_cast<int>(get_number_of_configurations())
                  && i >= -1,
                  "Invalid configuration requested.", IndexException);
  set_base();
  if (i == -1) return;
  const Diff &d= configurations_[i];
  // do something
  for (DiffMap::const_iterator it= d.diffs_.begin();
       it != d.diffs_.end(); ++it) {
    it->second.apply(it->first);
  }
  for (DataMap::const_iterator it= d.added_.begin();
       it != d.added_.end(); ++it) {
    model_->restore_particle(it->first);
    it->second.apply(it->first);
  }
  for (ParticleSet::const_iterator it= d.removed_.begin();
       it != d.removed_.end(); ++it) {
    model_->remove_particle(*it);
  }
  if (model_->get_is_incremental()) {
    // things move far so incremental won't be a good idea
    model_->set_is_incremental(false);
    model_->set_is_incremental(true);
  }
}


void ConfigurationSet::show(std::ostream &out) const {
  out << "ConfigurationSet with " << get_number_of_configurations()
      << " configurations." << std::endl;
}

IMP_END_NAMESPACE
