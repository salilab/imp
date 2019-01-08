/**
 *  \file IMP/atom/StructureSource.h     \brief Add a Structure ID and Chain ID
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_STRUCTURE_SOURCE_H
#define IMPATOM_STRUCTURE_SOURCE_H

#include <IMP/atom/atom_config.h>

#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>
#include <IMP/exception.h>

IMPATOM_BEGIN_NAMESPACE

//! Store strings describing the source of this structure fragment

class IMPATOMEXPORT StructureSource : public Decorator {
  static StringKey get_source_id_key();
  static StringKey get_source_chain_id_key();
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                std::string source_id,
                                std::string source_chain_id) {
    IMP_USAGE_CHECK(!source_id.empty(), "The structure ID cannot be empty.");
    m->add_attribute(get_source_id_key(), pi, source_id);
    m->add_attribute(get_source_chain_id_key(), pi, source_chain_id);
  }

 public:
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return (m->get_has_attribute(get_source_id_key(), pi) &&
            m->get_has_attribute(get_source_chain_id_key(), pi));
  }
  //! Get the source ID
  std::string get_source_id() const {
    return get_particle()->get_value(get_source_id_key());
  }

  //! Get the source ID
  std::string get_source_chain_id() const {
    return get_particle()->get_value(get_source_chain_id_key());
  }

  //! Set the structure ID
  void set_source_id(std::string source_id) {
    IMP_USAGE_CHECK(!source_id.empty(), "The structure ID cannot be empty");
    get_particle()->set_value(get_source_id_key(), source_id);
  }

  //! Set the structure ID
  void set_source_chain_id(std::string source_chain_id) {
    get_particle()->set_value(get_source_chain_id_key(), source_chain_id);
  }

  /* Declare the basic constructors and the cast function.*/
  IMP_DECORATOR_METHODS(StructureSource, Decorator);
  IMP_DECORATOR_SETUP_2(StructureSource, std::string, source_id,
                        std::string, source_chain_id);
};

IMP_DECORATORS(StructureSource, StructureSources, Particles);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_STRUCTURE_SOURCE_H */
