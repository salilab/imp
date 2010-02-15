/**
 *  \file ConfigurationSet.h
 *  \brief Store a set of configuration of the model.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_CONFIGURATION_SET_H
#define IMP_CONFIGURATION_SET_H

#include "config.h"
#include "Object.h"
#include "Pointer.h"
#include "Model.h"
#include "internal/particle_save.h"
#include <map>
#include <set>

IMP_BEGIN_NAMESPACE

//! A class to store a set of configurations of a model
/** The class maintains a set of structures found by a Sampler
    protocol and allows them to be queried and loaded and saved.
*/
class IMPEXPORT ConfigurationSet: public Object
{
  Pointer<Model> model_;
  typedef Pointer<Particle> PP;
  typedef std::map<PP, internal::ParticleData> DataMap;
  typedef std::map<PP, internal::ParticleDiff> DiffMap;
  typedef std::set<Particle*> ParticleSet;
  DataMap base_;
  struct Diff {
    DiffMap diffs_;
    DataMap added_;
    ParticleSet removed_;
  };
  std::vector<Diff> configurations_;

  void set_base();
 public:
  ConfigurationSet(Model *m, std::string name="ConfigurationSet %1%");
  //! Save the current configuration of the Model
  void save_configuration();
  unsigned int get_number_of_configurations() const;
  //! Load the ith configuration into the Model
  /** Passing -1 returns it to the base configuration. */
  void set_configuration(int i);
  IMP_OBJECT(ConfigurationSet);
};



IMP_END_NAMESPACE

#endif  /* IMP_CONFIGURATION_SET_H */
