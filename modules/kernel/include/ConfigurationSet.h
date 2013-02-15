/**
 *  \file IMP/kernel/ConfigurationSet.h
 *  \brief Store a set of configurations of the model.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_CONFIGURATION_SET_H
#define IMPKERNEL_CONFIGURATION_SET_H

#include <IMP/kernel/kernel_config.h>
#include "Object.h"
#include "Pointer.h"
#include "Model.h"
#include "Configuration.h"
#include "internal/OwnerPointer.h"
#include "OptimizerState.h"
#include "optimizer_state_macros.h"
#include "internal/utility.h"
#include <IMP/base/warning_macros.h>

IMPKERNEL_BEGIN_NAMESPACE


//! A class to store a set of configurations of a model
/** The class maintains a set of structures found by a Sampler
    protocol and allows them to be queried and loaded and saved.
    \pythonexample{basic_optimization}
*/
class IMPKERNELEXPORT ConfigurationSet: public IMP::base::Object
{
  mutable Pointer<Model> model_;
  Pointer<Configuration> base_;
  base::Vector<Pointer<Configuration> > configurations_;
 public:
  ConfigurationSet(Model *m, std::string name="ConfigurationSet %1%");
  //! Save the current configuration of the Model
  void save_configuration();
  unsigned int get_number_of_configurations() const;
  //! Load the ith configuration into the Model
  /** Passing -1 returns it to the base configuration. */
  void load_configuration(int i) const;
  void remove_configuration(unsigned int i);
  Model *get_model() const {
    return model_;
  }
  IMP_OBJECT_METHODS(ConfigurationSet);
};

IMP_OBJECTS(ConfigurationSet,ConfigurationSets);

IMP_MODEL_SAVE(SaveToConfigurationSet,
               (ConfigurationSet *cs, std::string file_name),
               mutable OwnerPointer<ConfigurationSet> cs_;,
               cs_=cs;,
               ,
               {
                 IMP_LOG_VARIABLE(file_name);
                 IMP_LOG_TERSE( "Saving to configuration set "
                         << file_name << std::endl);
                 cs_->save_configuration();
               });


IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_CONFIGURATION_SET_H */
