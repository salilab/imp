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

IMPATOM_END_NAMESPACE
