/**
 *  \file ConfiguratioNSet.cpp
 *  \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/ConfigurationSet.h"
#include "IMP/internal/utility.h"
#ifdef IMP_USE_NETCDF
#include <netcdfcpp.h>
#endif

IMP_BEGIN_NAMESPACE

ConfigurationSet::ConfigurationSet(Model *m,
                                   std::string nm):
  Object(nm),
  model_(m), base_(new Configuration(m, nm +" base")){
}


void ConfigurationSet::save_configuration() {
  IMP_OBJECT_LOG;
  set_was_used(true);
  IMP_LOG(TERSE, "Adding configuration to set " << get_name() << std::endl);
  configurations_.push_back(Diff());
  for (Model::ParticleIterator it= model_->particles_begin();
       it != model_->particles_end(); ++it) {
    PP pp(*it);
    if (base_->get_has_particle(pp)) {
      configurations_.back().diffs_[pp]
        = internal::ParticleDiff(base_->get_data(pp), pp);
    } else {
      configurations_.back().added_[pp]= internal::ParticleData(pp);
    }
  }
  for (Configuration::iterator it= base_->begin(); it != base_->end(); ++it) {
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

void ConfigurationSet::load_configuration(int i) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(this);
  IMP_USAGE_CHECK(i < static_cast<int>(get_number_of_configurations())
                  && i >= -1,
                  "Invalid configuration requested.");
  base_->load_configuration();
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


void ConfigurationSet::do_show(std::ostream &out) const {
  out <<  get_number_of_configurations()
      << " configurations." << std::endl;
}

#ifdef IMP_USE_NETCDF
// in io.cpp
void read_binary_model(NcFile &f,
                       const ParticlesTemp &particles,
                       const FloatKeys &keys,
                       int var_index);

ConfigurationSet* read_configuration_set(std::string filename,
                                         const Particles &ps,
                                         const FloatKeys &keys) {
  if (ps.empty()) {
    return NULL;
  }
  NcFile f(filename.c_str(), NcFile::ReadOnly,
           NULL, 0, NcFile::Netcdf4);
  if (!f.is_valid()) {
    IMP_THROW("Unable to open file " << filename << " for reading",
              IOException);
  }
  IMP_NEW(ConfigurationSet, ret, (ps[0]->get_model()));
  for (int i=0; i< f.num_vars(); ++i) {
    read_binary_model(f, ps, keys, i);
    ret->save_configuration();
  }
  return ret.release();
}
#endif

IMP_END_NAMESPACE
