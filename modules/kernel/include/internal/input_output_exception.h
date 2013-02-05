/**
 *  \file IMP/base/exception.h
 *  \brief Exception definitions and assertions.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INPUT_OUTPUT_EXCEPTION_H
#define IMPKERNEL_INPUT_OUTPUT_EXCEPTION_H

#include <IMP/kernel/kernel_config.h>
#include <IMP/base/exception.h>
IMPKERNEL_BEGIN_NAMESPACE
class ModelObject;
IMPKERNEL_END_NAMESPACE

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

//! An exception when something is missing from an input or output list.
/** See IMP::ModelObject::get_inputs() and IMP::ModelObject::get_outputs().
 */
struct IMPKERNELEXPORT InputOutputException: public std::runtime_error
{
  InputOutputException(int particle_index,
                       int operation,
                       int entity,
                       std::string key_name);
  InputOutputException(std::string container_name,
                       int entity);
   ~InputOutputException() throw();
  enum AccessEntity {NO_ENTITY, ATTRIBUTE, DERIVATIVE};
  enum AccessType {GET, SET, REMOVE, ADD};
  int get_particle_index() const {return particle_index_;};
  std::string get_container_name() const {return container_name_;};
  int get_operation() const {return operation_;}
  int get_entity() const {return entity_;}
  std::string get_key_name() const {return key_name_;}
  std::string get_message(ModelObject *o) const;
private:
  int particle_index_;
  std::string container_name_;
  int operation_;
  int entity_;
  std::string key_name_;
};


IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INPUT_OUTPUT_EXCEPTION_H */
