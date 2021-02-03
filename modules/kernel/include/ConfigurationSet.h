/**
 *  \file IMP/ConfigurationSet.h
 *  \brief Store a set of configurations of the model.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_CONFIGURATION_SET_H
#define IMPKERNEL_CONFIGURATION_SET_H

#include <IMP/kernel_config.h>
#include <IMP/Object.h>
#include <IMP/Pointer.h>
#include "Model.h"
#include "Configuration.h"
#include "OptimizerState.h"
#include "internal/utility.h"
#include <IMP/warning_macros.h>

IMPKERNEL_BEGIN_NAMESPACE

//! A class to store a set of configurations of a model
/** The class maintains a set of structures found by a Sampler
    protocol and allows them to be queried and loaded and saved.
    \include basic_optimization.py
*/
class IMPKERNELEXPORT ConfigurationSet : public IMP::Object {
  mutable Pointer<Model> model_;
  Pointer<Configuration> base_;
  Vector<Pointer<Configuration> > configurations_;

 public:
  ConfigurationSet(Model *m, std::string name = "ConfigurationSet %1%");
  //! Save the current configuration of the Model
  void save_configuration();
  unsigned int get_number_of_configurations() const;
  //! Load the ith configuration into the Model
  /** Passing -1 returns it to the base configuration. */
  void load_configuration(int i) const;
  void remove_configuration(unsigned int i);
  Model *get_model() const { return model_; }
  IMP_OBJECT_METHODS(ConfigurationSet);
};

IMP_OBJECTS(ConfigurationSet, ConfigurationSets);

/** Save the model to a ConfigurationSet. */
class IMPKERNELEXPORT SaveToConfigurationSetOptimizerState
    : public OptimizerState {
  PointerMember<ConfigurationSet> cs_;

 public:
  SaveToConfigurationSetOptimizerState(ConfigurationSet *cs);

 protected:
  virtual void do_update(unsigned int update_number) IMP_OVERRIDE;
  IMP_OBJECT_METHODS(SaveToConfigurationSetOptimizerState);
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_CONFIGURATION_SET_H */
