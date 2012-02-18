/**
 *  \file Configuration.h
 *  \brief Store a set of configurations of the model.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_CONFIGURATION_H
#define IMPKERNEL_CONFIGURATION_H

#include "kernel_config.h"
#include "Object.h"
#include "Pointer.h"
#include "Model.h"
#include "compatibility/map.h"

IMP_BEGIN_NAMESPACE


//! A class to store a configuration of a model
/** */
class IMPEXPORT Configuration: public IMP::base::Object
{
  mutable Pointer<Model> model_;
  internal::FloatAttributeTable floats_;
  internal::StringAttributeTable strings_;
  internal::IntAttributeTable objects_;
  internal::ObjectAttributeTable ints_lists_;
  internal::IntsAttributeTable objects_lists_;
  internal::ObjectsAttributeTable particles_;
  internal::ParticleAttributeTable particles_lists_;
  internal::ParticlesAttributeTable ints_;
 public:
  Configuration(Model *m, std::string name="Configuration %1%");
  void load_configuration() const;
  //! Swap the current configuration with that in the Model
  /** This should be faster than loading (or at least not slower.
   */
  void swap_configuration();
  IMP_OBJECT(Configuration);
};

IMP_OBJECTS(Configuration,Configurations);

IMP_END_NAMESPACE

#endif  /* IMPKERNEL_CONFIGURATION_H */
