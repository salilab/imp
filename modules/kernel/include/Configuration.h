/**
 *  \file IMP/Configuration.h
 *  \brief Store a set of configurations of the model.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_CONFIGURATION_H
#define IMPKERNEL_CONFIGURATION_H

#include <IMP/kernel_config.h>
#include <IMP/Object.h>
#include <IMP/Pointer.h>
#include "Model.h"
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPKERNEL_BEGIN_NAMESPACE

//! A class to store a configuration of a model
class IMPKERNELEXPORT Configuration : public IMP::Object {
  mutable Pointer<Model> model_;
  Pointer<Object> base_;
  internal::FloatAttributeTable floats_;
  internal::FloatsAttributeTable float_lists_;
  internal::StringAttributeTable strings_;
  internal::IntAttributeTable ints_;
  internal::ObjectAttributeTable objects_;
  internal::WeakObjectAttributeTable weak_objects_;
  internal::IntsAttributeTable int_lists_;
  internal::ObjectsAttributeTable object_lists_;
  internal::ParticleAttributeTable particles_;
  internal::ParticlesAttributeTable particle_lists_;
  internal::Vector3DAttributeTable vector3ds_;
  internal::Vector4DAttributeTable vector4ds_;
  internal::SparseStringAttributeTable sparse_strings_;
  internal::SparseIntAttributeTable sparse_ints_;
  internal::SparseFloatAttributeTable sparse_floats_;
  internal::SparseParticleAttributeTable sparse_particles_;
  internal::Vector3DDerivAttributeTable vector3d_derivs_;
  internal::Vector4DDerivAttributeTable vector4d_derivs_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Object>(this));
    if (std::is_base_of<cereal::detail::OutputArchiveBase, Archive>::value) {
      uint32_t model_id = get_model_id();
      ar(model_id);
    } else {
      uint32_t model_id;
      ar(model_id);
      set_model_from_id(model_id);
    }
    ar(base_, floats_, float_lists_, strings_, ints_, objects_, weak_objects_,
       int_lists_, object_lists_, particles_, particle_lists_, vector3ds_,
       vector4ds_, sparse_strings_, sparse_ints_, sparse_floats_,
       sparse_particles_, vector3d_derivs_, vector4d_derivs_);
  }
  void set_model_from_id(uint32_t model_id);
  uint32_t get_model_id() const;

 public:
  Configuration(Model *m, std::string name = "Configuration %1%");
  //! Only store parts of the configuration that have changed from base
  /** At the moment, this does not play well with adding and removing
      attributes.*/
  Configuration(Model *m, Configuration *base,
                std::string name = "Configuration %1%");

  Configuration() : Object("") {}

  void load_configuration() const;
  //! Swap the current configuration with that in the Model
  /** This should be faster than loading (or at least not slower).
   */
  void swap_configuration();
  IMP_OBJECT_METHODS(Configuration);
};

IMP_OBJECTS(Configuration, Configurations);

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_CONFIGURATION_H */
