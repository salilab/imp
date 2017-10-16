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

std::set<std::string>& SampleProvenance::get_allowed_methods() {
  static std::set<std::string> m;
  if (m.empty()) {
    m.insert("Monte Carlo");
    m.insert("Molecular Dynamics");
  }
  return m;
}

StringKey SampleProvenance::get_method_key() {
  static const StringKey method("sp_method");
  return method;
}

IntKey SampleProvenance::get_frames_key() {
  static const IntKey frames("sp_frames");
  return frames;
}

IntKey SampleProvenance::get_iterations_key() {
  static const IntKey iterations("sp_iterations");
  return iterations;
}

void SampleProvenance::show(std::ostream &out) const {
  out << "SampleProvenance " << get_number_of_frames() << " of "
      << get_method() << std::endl;
}

IntKey CombineProvenance::get_runs_key() {
  static const IntKey runs("cp_runs");
  return runs;
}

IntKey CombineProvenance::get_frames_key() {
  static const IntKey frames("cp_frames");
  return frames;
}

void CombineProvenance::show(std::ostream &out) const {
  out << "CombineProvenance of " << get_number_of_runs()
      << " runs resulting in " << get_number_of_frames()
      << " frames" << std::endl;
}

IntKey ClusterProvenance::get_members_key() {
  static const IntKey members("cp_members");
  return members;
}

void ClusterProvenance::show(std::ostream &out) const {
  out << "ClusterProvenance with " << get_number_of_members()
      << " members" << std::endl;
}

FloatKey FilterProvenance::get_threshold_key() {
  static const FloatKey threshold("fp_threshold");
  return threshold;
}

IntKey FilterProvenance::get_frames_key() {
  static const IntKey frames("fp_frames");
  return frames;
}

void FilterProvenance::show(std::ostream &out) const {
  out << "FilterProvenance threshold " << get_threshold()
      << " resulting in " << get_number_of_frames() << " frames" << std::endl;
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
