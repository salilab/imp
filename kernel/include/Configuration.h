/**
 *  \file Configuration.h
 *  \brief Store a set of configurations of the model.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_CONFIGURATION_H
#define IMP_CONFIGURATION_H

#include "kernel_config.h"
#include "Object.h"
#include "Pointer.h"
#include "Model.h"
#include "internal/particle_save.h"
#include <map>
#include <set>

IMP_BEGIN_NAMESPACE


//! A class to store a configuration of a model
/** */
class IMPEXPORT Configuration: public Object
{
  mutable Pointer<Model> model_;
  typedef Pointer<Particle> PP;
  typedef std::map<PP, internal::ParticleData> DataMap;
  DataMap base_;
  friend class ConfigurationSet;
  bool get_has_particle(Particle *p) const {
    return base_.find(p) != base_.end();
  }
  const internal::ParticleData& get_data(Particle *p) const {
    return base_.find(p)->second;
  }
  typedef DataMap::const_iterator iterator;
  iterator begin() const {return base_.begin();}
  iterator end() const {return base_.end();}
 public:
  Configuration(Model *m, std::string name="Configuration %1%");
  void load_configuration() const;
  IMP_OBJECT(Configuration);
};

IMP_OBJECTS(Configuration,Configurations);

IMP_END_NAMESPACE

#endif  /* IMP_CONFIGURATION_H */
