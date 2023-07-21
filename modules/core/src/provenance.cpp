/**
 *  \file provenance.cpp
 *  \brief Classes to track how the model was created.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
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

SparseParticleIndexKey Provenanced::get_provenance_key() {
  static const SparseParticleIndexKey provenance("provenance");
  return provenance;
}

void Provenanced::show(std::ostream &out) const {
  out << "Provenanced" << std::endl;
}

SparseParticleIndexKey Provenance::get_previous_key() {
  static const SparseParticleIndexKey previous("previous_provenance");
  return previous;
}

void Provenance::show(std::ostream &out) const {
  out << "Provenance" << std::endl;
}

SparseStringKey StructureProvenance::get_filename_key() {
  static const SparseStringKey filename("sp_filename");
  return filename;
}

SparseStringKey StructureProvenance::get_chain_key() {
  static const SparseStringKey chain("sp_chain");
  return chain;
}

SparseIntKey StructureProvenance::get_residue_offset_key() {
  static const SparseIntKey offset("sp_offset");
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

SparseStringKey SampleProvenance::get_method_key() {
  static const SparseStringKey method("sp_method");
  return method;
}

SparseIntKey SampleProvenance::get_frames_key() {
  static const SparseIntKey frames("sp_frames");
  return frames;
}

SparseIntKey SampleProvenance::get_iterations_key() {
  static const SparseIntKey iterations("sp_iterations");
  return iterations;
}

SparseIntKey SampleProvenance::get_replicas_key() {
  static const SparseIntKey replicas("sp_replicas");
  return replicas;
}

void SampleProvenance::show(std::ostream &out) const {
  out << "SampleProvenance " << get_number_of_frames() << " of "
      << get_method() << std::endl;
}

SparseIntKey CombineProvenance::get_runs_key() {
  static const SparseIntKey runs("cp_runs");
  return runs;
}

SparseIntKey CombineProvenance::get_frames_key() {
  static const SparseIntKey frames("cp_frames");
  return frames;
}

void CombineProvenance::show(std::ostream &out) const {
  out << "CombineProvenance of " << get_number_of_runs()
      << " runs resulting in " << get_number_of_frames()
      << " frames" << std::endl;
}

SparseIntKey ClusterProvenance::get_members_key() {
  static const SparseIntKey members("cp_members");
  return members;
}

SparseFloatKey ClusterProvenance::get_precision_key() {
  static const SparseFloatKey precision("cp_precision");
  return precision;
}

SparseStringKey ClusterProvenance::get_density_key() {
  static const SparseStringKey density("cp_density");
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

SparseStringKey FilterProvenance::get_method_key() {
  static const SparseStringKey method("fp_method");
  return method;
}

SparseFloatKey FilterProvenance::get_threshold_key() {
  static const SparseFloatKey threshold("fp_threshold");
  return threshold;
}

SparseIntKey FilterProvenance::get_frames_key() {
  static const SparseIntKey frames("fp_frames");
  return frames;
}

void FilterProvenance::show(std::ostream &out) const {
  out << "FilterProvenance threshold " << get_threshold()
      << " resulting in " << get_number_of_frames() << " frames" << std::endl;
}

SparseStringKey ScriptProvenance::get_filename_key() {
  static const SparseStringKey filename("script_filename");
  return filename;
}

void ScriptProvenance::show(std::ostream &out) const {
  out << "ScriptProvenance " << get_filename() << std::endl;
}

SparseStringKey SoftwareProvenance::get_name_key() {
  static const SparseStringKey name("software_name");
  return name;
}

SparseStringKey SoftwareProvenance::get_version_key() {
  static const SparseStringKey version("software_version");
  return version;
}

SparseStringKey SoftwareProvenance::get_location_key() {
  static const SparseStringKey location("software_location");
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
