/**
 *  \file IMP/kernel/Configuration.h
 *  \brief Store a set of configurations of the model.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_CONFIGURATION_H
#define IMPKERNEL_CONFIGURATION_H

#include <IMP/kernel/kernel_config.h>
#include "Object.h"
#include "Pointer.h"
#include "Model.h"
#include <IMP/base/map.h>

IMPKERNEL_BEGIN_NAMESPACE


//! A class to store a configuration of a model
/** */
class IMPKERNELEXPORT Configuration: public IMP::base::Object
{
  mutable Pointer<Model> model_;
  Pointer<Object> base_;
  internal::FloatAttributeTable floats_;
  internal::StringAttributeTable strings_;
  internal::IntAttributeTable ints_;
  internal::ObjectAttributeTable objects_;
  internal::WeakObjectAttributeTable weak_objects_;
  internal::IntsAttributeTable int_lists_;
  internal::ObjectsAttributeTable object_lists_;
  internal::ParticleAttributeTable particles_;
  internal::ParticlesAttributeTable particle_lists_;
 public:
  Configuration(Model *m, std::string name="Configuration %1%");
  //! Only store parts of the configuration that have changed from base
  /** At the moment, this does not play well with adding and removing
      attributes.*/
  Configuration(Model *m, Configuration *base,
                std::string name="Configuration %1%");
  void load_configuration() const;
  //! Swap the current configuration with that in the Model
  /** This should be faster than loading (or at least not slower.
   */
  void swap_configuration();
  IMP_OBJECT(Configuration);
};

IMP_OBJECTS(Configuration,Configurations);

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_CONFIGURATION_H */
