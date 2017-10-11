/**
 *  \file provenance.cpp
 *  \brief Classes to track how the model was created.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 */

#include <IMP/atom/provenance.h>

IMPATOM_BEGIN_NAMESPACE

ParticleIndexKey Provenanced::get_provenance_key() {
  static const ParticleIndexKey provenance("provenance");
  return provenance;
}

void Provenanced::show(std::ostream &out) const {
  out << "Provenanced" << std::endl;
}

ParticleIndexKey Provenance::get_previous_state_key() {
  static const ParticleIndexKey previous_state("previous_state");
  return previous_state;
}

void Provenance::show(std::ostream &out) const {
  out << "Provenance" << std::endl;
}

StringKey StructureProvenance::get_filename_key() {
  static const StringKey filename("sp_filename");
  return filename;
}

StringKey StructureProvenance::get_chain_key() {
  static const StringKey chain("sp_chain");
  return chain;
}

void StructureProvenance::show(std::ostream &out) const {
  out << "StructureProvenance " << get_filename() << " " << get_chain_id()
      << std::endl;
}

void add_provenance(Model *m, ParticleIndex pi, Provenance p) {
  if (Provenanced::get_is_setup(m, pi)) {
    // add the new provenance as a new root
    Provenanced pd = Provenanced(m, pi);
    Provenance old_provenance = pd.get_provenance();
    p.set_previous_state(old_provenance);
    pd.set_provenance(p);
  } else {
    Provenanced::setup_particle(m, pi, p);
  }
}

IMPATOM_END_NAMESPACE
