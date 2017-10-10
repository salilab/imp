/**
 *  \file IMP/atom/provenance.h
 *  \brief Classes to track how the model was created.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_PROVENANCE_H
#define IMPATOM_PROVENANCE_H

#include <IMP/atom/atom_config.h>

#include <IMP/base_types.h>
#include <IMP/Object.h>
#include <IMP/object_macros.h>
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>

IMPATOM_BEGIN_NAMESPACE

//! Track how parts of the system were created.
/** Particles are linked with this decorator into a directed acyclic graph
    that tracks all IMP transformations of the system all the way back to
    raw inputs (such as PDB files).

    Typically, part of an IMP::Model (usually an atom::Hierarchy particle)
    is decorated with Provenanced that points to the root of this graph.
 */
class IMPATOMEXPORT Provenance : public Decorator {
  static void do_setup_particle(Model *m, ParticleIndex pi) {
    // Use self-index to indicate no previous state is set yet
    m->add_attribute(get_previous_state_key(), pi, pi);
  }

  static ParticleIndexKey get_previous_state_key();

public:
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_previous_state_key(), pi);
  }

  //! \return the previous state, or Provenance() if none exists.
  Provenance get_previous_state() const {
    ParticleIndex pi = get_model()->get_attribute(get_previous_state_key(),
                                                  get_particle_index());
    // self-index indicates no previous state is set yet
    if (pi == get_particle_index()) {
      return Provenance();
    } else {
      return Provenance(get_model(), pi);
    }
  }

  //! Set the previous state
  /** It is considered an error to try to set this more than once. */
  void set_previous_state(Provenance p) {
    IMP_USAGE_CHECK(get_model()->get_attribute(get_previous_state_key(),
                                               get_particle_index())
                            == get_particle_index(),
                    "Previous state is already set");
    get_model()->set_attribute(get_previous_state_key(),
                               get_particle_index(), p.get_particle_index());
  }

  IMP_DECORATOR_METHODS(Provenance, Decorator);
  IMP_DECORATOR_SETUP_0(Provenance);
};

//! Track creation of a system fragment from a PDB file.
class IMPATOMEXPORT StructureProvenance : public Provenance {
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                std::string filename,
                                std::string chain_id) {
    IMP_USAGE_CHECK(!filename.empty(), "The filename cannot be empty.");
    m->add_attribute(get_filename_key(), pi, filename);
    m->add_attribute(get_chain_key(), pi, chain_id);
  }

  static StringKey get_filename_key();
  static StringKey get_chain_key();

public:
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_filename_key(), pi)
           && m->get_has_attribute(get_chain_key(), pi);
  }

  //! Set the filename
  void set_filename(std::string filename) const {
    IMP_USAGE_CHECK(!filename.empty(), "The filename cannot be empty");
    return get_model()->set_attribute(get_filename_key(), get_particle_index(),
                                      filename);
  }

  //! \return the filename
  std::string get_filename() const {
    return get_model()->get_attribute(get_filename_key(), get_particle_index());
  }

  //! Set the chain ID
  void set_chain_id(std::string chain_id) const {
    return get_model()->set_attribute(get_chain_key(), get_particle_index(),
                                      chain_id);
  }

  //! \return the chain ID
  std::string get_chain_id() const {
    return get_model()->get_attribute(get_chain_key(), get_particle_index());
  }

  IMP_DECORATOR_METHODS(StructureProvenance, Provenance);
  IMP_DECORATOR_SETUP_2(StructureProvenance, std::string, filename,
                        std::string, chain_id);
};

//! Tag part of the system to track how it was created.
class IMPATOMEXPORT Provenanced : public Decorator {
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                Provenance p) {
    m->add_attribute(get_provenance_key(), pi, p.get_particle_index());
  }

  static ParticleIndexKey get_provenance_key();
public:

  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_provenance_key(), pi);
  }

  Provenance get_provenance() const {
    ParticleIndex pi = get_model()->get_attribute(get_provenance_key(),
                                                  get_particle_index());
    return Provenance(get_model(), pi);
  }

  void set_provenance(Provenance p) const {
    get_model()->set_attribute(get_provenance_key(), get_particle_index(),
                               p.get_particle_index());
  }

  IMP_DECORATOR_METHODS(Provenanced, Decorator);
  IMP_DECORATOR_SETUP_1(Provenanced, Provenance, p);
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_PROVENANCE_H */
