/**
 *  \file provenance.cpp
 *  \brief Classes to track how the model was created.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 */

#include <IMP/core/provenance.h>

IMPCORE_BEGIN_NAMESPACE

namespace {

Provenance clone_one(Provenance prov) {
  Particle *p = new IMP::Particle(prov.get_model());
  p->set_name(prov->get_name());

  if (StructureProvenance::get_is_setup(prov.get_particle())) {
    StructureProvenance::setup_particle(p,
                                StructureProvenance(prov.get_particle()));
  } else if (SampleProvenance::get_is_setup(prov.get_particle())) {
    SampleProvenance::setup_particle(p,
                                SampleProvenance(prov.get_particle()));
  } else if (CombineProvenance::get_is_setup(prov.get_particle())) {
    CombineProvenance::setup_particle(p,
                                CombineProvenance(prov.get_particle()));
  } else if (FilterProvenance::get_is_setup(prov.get_particle())) {
    FilterProvenance::setup_particle(p,
                                FilterProvenance(prov.get_particle()));
  } else if (ClusterProvenance::get_is_setup(prov.get_particle())) {
    ClusterProvenance::setup_particle(p,
                                ClusterProvenance(prov.get_particle()));
  } else if (ScriptProvenance::get_is_setup(prov.get_particle())) {
    ScriptProvenance::setup_particle(p,
                                ScriptProvenance(prov.get_particle()));
  } else if (SoftwareProvenance::get_is_setup(prov.get_particle())) {
    SoftwareProvenance::setup_particle(p,
                                SoftwareProvenance(prov.get_particle()));
  } else {
    IMP_THROW("Unhandled provenance", IOException);
  }
  return Provenance(p);
}

} // anonymous namespace

ParticleIndexKey Provenanced::get_provenance_key() {
  static const ParticleIndexKey provenance("provenance");
  return provenance;
}

void Provenanced::show(std::ostream &out) const {
  out << "Provenanced" << std::endl;
}

ParticleIndexKey Provenance::get_previous_key() {
  static const ParticleIndexKey previous("previous_provenance");
  return previous;
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

IntKey StructureProvenance::get_residue_offset_key() {
  static const IntKey offset("sp_offset");
  return offset;
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
    m.insert("Hybrid MD/MC");
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

IntKey SampleProvenance::get_replicas_key() {
  static const IntKey replicas("sp_replicas");
  return replicas;
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

FloatKey ClusterProvenance::get_precision_key() {
  static const FloatKey precision("cp_precision");
  return precision;
}

StringKey ClusterProvenance::get_density_key() {
  static const StringKey density("cp_density");
  return density;
}

void ClusterProvenance::show(std::ostream &out) const {
  out << "ClusterProvenance with " << get_number_of_members()
      << " members, at precision " << get_precision() << std::endl;
}

std::set<std::string>& FilterProvenance::get_allowed_methods() {
  static std::set<std::string> m;
  if (m.empty()) {
    m.insert("Total score");
    m.insert("Best scoring");
    m.insert("Keep fraction");
  }
  return m;
}

StringKey FilterProvenance::get_method_key() {
  static const StringKey method("fp_method");
  return method;
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

StringKey ScriptProvenance::get_filename_key() {
  static const StringKey filename("script_filename");
  return filename;
}

void ScriptProvenance::show(std::ostream &out) const {
  out << "ScriptProvenance " << get_filename() << std::endl;
}

StringKey SoftwareProvenance::get_name_key() {
  static const StringKey name("software_name");
  return name;
}

StringKey SoftwareProvenance::get_version_key() {
  static const StringKey version("software_version");
  return version;
}

StringKey SoftwareProvenance::get_location_key() {
  static const StringKey location("software_location");
  return location;
}

void SoftwareProvenance::show(std::ostream &out) const {
  out << "SoftwareProvenance " << get_software_name() << " version "
      << get_version() << " at " << get_location() << std::endl;
}

void add_provenance(Model *m, ParticleIndex pi, Provenance p) {
  if (Provenanced::get_is_setup(m, pi)) {
    // add the new provenance as a new root
    Provenanced pd(m, pi);
    Provenance old_provenance = pd.get_provenance();
    p.set_previous(old_provenance);
    pd.set_provenance(p);
  } else {
    Provenanced::setup_particle(m, pi, p);
  }
}

Provenance create_clone(Provenance prov) {
  Provenance root = clone_one(prov);

  Provenance newprov = root;
  while (prov) {
    Provenance previous = prov.get_previous();
    if (previous) {
      Provenance newprevious = clone_one(previous);
      newprov.set_previous(newprevious);
      newprov = newprevious;
    }
    prov = previous;
  }
  return root;
}

IMPCORE_END_NAMESPACE
