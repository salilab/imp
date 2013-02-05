/**
 *  \file exception.cpp   \brief Check handling.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/internal/input_output_exception.h"
#include "IMP/kernel/ModelObject.h"
#include "IMP/kernel/Model.h"

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

InputOutputException::InputOutputException(int particle_index,
                                           int operation,
                                           int entity,
                                           std::string key_name):
  std::runtime_error("InputOutputException"),
  particle_index_(particle_index),
  operation_(operation),
  entity_(entity),
  key_name_(key_name) {
}

InputOutputException::InputOutputException(std::string container_name,
                                           int operation):
  std::runtime_error("InputOutputException"),
  particle_index_(-1),
  container_name_(container_name),
  operation_(operation),
  entity_(NO_ENTITY) {
}

InputOutputException::~InputOutputException() throw()
{
}

std::string InputOutputException::get_message(ModelObject *o) const {
  std::ostringstream oss;
  switch (get_entity()) {
    case DERIVATIVE:
    if (o->get_model()->get_stage()== BEFORE_EVALUATING) {
      oss << "Derivatives cannot be read before evaluating.";
      break;
    }
    default:
    switch(get_operation()) {
      case GET:
      oss << "Not in input list.";
      break;
      case SET:
      case ADD:
      case REMOVE:
      oss << "Not in output list.";
      break;
      default:
                    // should not exist
      oss << "Unknown read/write error";
    }
    break;
  };
  oss << " Violating object: \"" << o->get_name() << "\".";
  if (particle_index_>=0) {
    oss << " Attribute " << get_key_name()
    << " of particle \""
    << o->get_model()->get_particle(ParticleIndex(get_particle_index()))
    ->get_name()
    << "\" with id " << get_particle_index();
  } else {
    oss << "Container \"" << container_name_ << "\".";
  }
  return oss.str();
}

IMPKERNEL_END_INTERNAL_NAMESPACE
