/**
 *  \file IMP/core/provenance.h
 *  \brief Classes to track how the model was created.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_PROVENANCE_H
#define IMPCORE_PROVENANCE_H

#include <IMP/core/core_config.h>

#include <IMP/base_types.h>
#include <IMP/Object.h>
#include <IMP/object_macros.h>
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>
#include <IMP/file.h>
#include <set>

IMPCORE_BEGIN_NAMESPACE

//! Track how parts of the system were created.
/** Particles are linked with this decorator into a directed acyclic graph
    that tracks all IMP transformations of the system all the way back to
    raw inputs (such as PDB files).

    Typically, part of an IMP::Model (usually an atom::Hierarchy particle)
    is decorated with Provenanced that points to the root of this graph.
 */
class IMPCOREEXPORT Provenance : public Decorator {
  static void do_setup_particle(Model *m, ParticleIndex pi) {
    // Use self-index to indicate no previous provenance is set yet
    m->add_attribute(get_previous_key(), pi, pi);
  }

  static ParticleIndexKey get_previous_key();

public:
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_previous_key(), pi);
  }

  //! \return the previous provenance, or Provenance() if none exists.
  Provenance get_previous() const {
    ParticleIndex pi = get_model()->get_attribute(get_previous_key(),
                                                  get_particle_index());
    // self-index indicates no previous provenance is set yet
    if (pi == get_particle_index()) {
      return Provenance();
    } else {
      return Provenance(get_model(), pi);
    }
  }

  //! Set the previous provenance.
  /** This can be used to show that a given part of the system was
      generated through multiple steps in order, for example by first
      being read from a PDB file, then sampled, filtered, and finally
      clustered.

      \note it is considered an error to try to set this more than once.
    */
  void set_previous(Provenance p) {
    IMP_USAGE_CHECK(get_model()->get_attribute(get_previous_key(),
                                               get_particle_index())
                            == get_particle_index(),
                    "Previous provenance is already set");
    get_model()->set_attribute(get_previous_key(),
                               get_particle_index(), p.get_particle_index());
  }

  IMP_DECORATOR_METHODS(Provenance, Decorator);
  IMP_DECORATOR_SETUP_0(Provenance);
};

//! Track creation of a system fragment from a PDB file.
/** This tracks the filename of the PDB file, and the chain ID,
    that was used to populate an IMP Model (if multiple chains are
    read, IMP::atom::read_pdb() assigns a StructureProvenance to each
    chain). The offset between the residue indexes in the PDB file and the
    Model is also recorded. Normally this is zero, but can be used to indicate
    that residues were renumbered, e.g. to match canonical numbering.
    The convention is PDB residue # - offset = Model residue #.
 */
class IMPCOREEXPORT StructureProvenance : public Provenance {
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                std::string filename,
                                std::string chain_id,
                                int residue_offset=0) {
    Provenance::setup_particle(m, pi);
    IMP_USAGE_CHECK(!filename.empty(), "The filename cannot be empty.");
    m->add_attribute(get_filename_key(), pi, get_absolute_path(filename));
    m->add_attribute(get_chain_key(), pi, chain_id);
    m->add_attribute(get_residue_offset_key(), pi, residue_offset);
  }

  static void do_setup_particle(Model *m, ParticleIndex pi,
                                StructureProvenance o) {
    do_setup_particle(m, pi, o.get_filename(), o.get_chain_id(),
                      o.get_residue_offset());
  }

  static StringKey get_filename_key();
  static StringKey get_chain_key();
  static IntKey get_residue_offset_key();

public:
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_filename_key(), pi)
           && m->get_has_attribute(get_chain_key(), pi)
           && m->get_has_attribute(get_residue_offset_key(), pi);
  }

  //! Set the filename
  /** The path can be relative or absolute. Internally, an absolute
      path will be stored (although generally it will be converted to
      a relative path when storing in a file, such as RMF).
    */
  void set_filename(std::string filename) const {
    IMP_USAGE_CHECK(!filename.empty(), "The filename cannot be empty");
    return get_model()->set_attribute(get_filename_key(), get_particle_index(),
                                      get_absolute_path(filename));
  }

  //! \return the filename, as an absolute path
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

  //! Set the offset between PDB and internal numbering (defaults to 0)
  void set_residue_offset(int residue_offset) const {
    return get_model()->set_attribute(get_residue_offset_key(),
                                      get_particle_index(),
                                      residue_offset);
  }

  //! \return the offset between PDB and internal numbering (defaults to 0)
  int get_residue_offset() const {
    return get_model()->get_attribute(get_residue_offset_key(),
                                      get_particle_index());
  }

  IMP_DECORATOR_METHODS(StructureProvenance, Provenance);
  IMP_DECORATOR_SETUP_3(StructureProvenance, std::string, filename,
                        std::string, chain_id, int, residue_offset);
  IMP_DECORATOR_SETUP_2(StructureProvenance, std::string, filename,
                        std::string, chain_id);
  IMP_DECORATOR_SETUP_1(StructureProvenance, StructureProvenance, o);
};

//! Track creation of a system fragment from sampling.
/** Part of the system (usually the top of a Hierarchy) tagged with this
    decorator is understood to be a single frame from an ensemble of
    multiple frames generated with some sampling method (e.g. Monte Carlo).
    Additionally, the number of iterations of the sampler used to generate
    each frame can be stored, if known and applicable.
    The rest of the frames are generally stored in a file (e.g. an RMF file).

    \throw UsageException if invalid sampling method
  */
class IMPCOREEXPORT SampleProvenance : public Provenance {
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                std::string method, int frames,
                                int iterations, int replicas=1) {
    validate_method(method);
    Provenance::setup_particle(m, pi);
    m->add_attribute(get_method_key(), pi, method);
    m->add_attribute(get_frames_key(), pi, frames);
    m->add_attribute(get_iterations_key(), pi, iterations);
    m->add_attribute(get_replicas_key(), pi, replicas);
  }

  static void do_setup_particle(Model *m, ParticleIndex pi,
                                SampleProvenance o) {
    do_setup_particle(m, pi, o.get_method(), o.get_number_of_frames(),
                      o.get_number_of_iterations(),
                      o.get_number_of_replicas());
  }

  static StringKey get_method_key();
  static IntKey get_frames_key();
  static IntKey get_iterations_key();
  static IntKey get_replicas_key();

  // get list of method names allowed in SamplingProvenance
  static std::set<std::string>& get_allowed_methods();

  //! validate specified sampling method is in get_allowed_methods().
  //! \throw IMP::UsageException if invalid
  static void validate_method(std::string method) {
    IMP_ALWAYS_CHECK(get_allowed_methods().find(method)
                           != get_allowed_methods().end(),
                           "Invalid sampling method",
                           IMP::UsageException);
  }

public:
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_method_key(), pi)
           && m->get_has_attribute(get_iterations_key(), pi)
           && m->get_has_attribute(get_frames_key(), pi);
  }

  //! Set the sampling method
  //! \throw IMP::UsageException invalid = not in get_allowed_methods().
  void set_method(std::string method) const {
    validate_method(method);
    return get_model()->set_attribute(get_method_key(), get_particle_index(),
                                      method);
  }

  //! \return the sampling method
  std::string get_method() const {
    return get_model()->get_attribute(get_method_key(), get_particle_index());
  }

  //! Set the number of frames
  void set_number_of_frames(int frames) const {
    return get_model()->set_attribute(get_frames_key(), get_particle_index(),
                                      frames);
  }

  //! \return the number of frames
  int get_number_of_frames() const {
    return get_model()->get_attribute(get_frames_key(), get_particle_index());
  }

  //! Set the number of iterations
  void set_number_of_iterations(int iterations) const {
    return get_model()->set_attribute(get_iterations_key(),
                                      get_particle_index(), iterations);
  }

  //! \return the number of iterations
  int get_number_of_iterations() const {
    return get_model()->get_attribute(get_iterations_key(),
                                      get_particle_index());
  }

  //! Set the number of replicas
  void set_number_of_replicas(int replicas) const {
    return get_model()->set_attribute(get_replicas_key(),
                                      get_particle_index(), replicas);
  }

  //! \return the number of replicas
  int get_number_of_replicas() const {
    return get_model()->get_attribute(get_replicas_key(),
                                      get_particle_index());
  }

  IMP_DECORATOR_METHODS(SampleProvenance, Provenance);
  IMP_DECORATOR_SETUP_4(SampleProvenance, std::string, method, int, frames,
                        int, iterations, int, replicas);
  IMP_DECORATOR_SETUP_3(SampleProvenance, std::string, method, int, frames,
                        int, iterations);
  IMP_DECORATOR_SETUP_1(SampleProvenance, SampleProvenance, o);
};

//! Track creation of a system fragment by combination.
/** Part of the system (usually the top of a Hierarchy) tagged with this
    decorator is understood to be a single frame within an ensemble that
    was created by combining a number of independent runs. One of those runs
    should be the 'previous' provenance. The runs should be
    essentially identical, differing at most only in the number of frames.
    The total size of the resulting ensemble is stored here.
  */
class IMPCOREEXPORT CombineProvenance : public Provenance {
  static void do_setup_particle(Model *m, ParticleIndex pi, int runs,
                                int frames) {
    Provenance::setup_particle(m, pi);
    m->add_attribute(get_runs_key(), pi, runs);
    m->add_attribute(get_frames_key(), pi, frames);
  }

  static void do_setup_particle(Model *m, ParticleIndex pi,
                                CombineProvenance o) {
    do_setup_particle(m, pi, o.get_number_of_runs(), o.get_number_of_frames());
  }

  static IntKey get_runs_key();
  static IntKey get_frames_key();

public:
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_frames_key(), pi)
           && m->get_has_attribute(get_runs_key(), pi);
  }

  //! Set the total number of frames
  void set_number_of_frames(int frames) const {
    return get_model()->set_attribute(get_frames_key(), get_particle_index(),
                                      frames);
  }

  //! \return the total number of frames
  int get_number_of_frames() const {
    return get_model()->get_attribute(get_frames_key(), get_particle_index());
  }

  //! Set the number of runs
  void set_number_of_runs(int runs) const {
    return get_model()->set_attribute(get_runs_key(), get_particle_index(),
                                      runs);
  }

  //! \return the number of runs
  int get_number_of_runs() const {
    return get_model()->get_attribute(get_runs_key(), get_particle_index());
  }

  IMP_DECORATOR_METHODS(CombineProvenance, Provenance);
  IMP_DECORATOR_SETUP_2(CombineProvenance, int, runs, int, frames);
  IMP_DECORATOR_SETUP_1(CombineProvenance, CombineProvenance, o);
};

//! Track creation of a system fragment by filtering.
/** Part of the system (usually the top of a Hierarchy) tagged with this
    decorator is understood to be a single frame within an ensemble that
    resulted from filtering a larger ensemble (the 'previous'
    provenance) in some fashion, such as
     - by discarding models with scores above the threshold;
     - by ranking the models and keeping the best scoring subset;
     - by keeping a fraction of models from the ensemble.

    \throw UsageException if method not in get_allowed_methods()
  */
class IMPCOREEXPORT FilterProvenance : public Provenance {
  static void do_setup_particle(Model *m, ParticleIndex pi, std::string method,
                                double threshold, int frames) {
    validate_method(method);
    Provenance::setup_particle(m, pi);
    m->add_attribute(get_method_key(), pi, method);
    m->add_attribute(get_threshold_key(), pi, threshold);
    m->add_attribute(get_frames_key(), pi, frames);
  }
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                FilterProvenance o) {
    do_setup_particle(m, pi, o.get_method(), o.get_threshold(),
                      o.get_number_of_frames());
  }

  static StringKey get_method_key();
  static FloatKey get_threshold_key();
  static IntKey get_frames_key();

  // get list of method names allowed in FilterProvenance
  static std::set<std::string>& get_allowed_methods();

  //! validate specified sampling method is in get_allowed_methods().
  //! \throw IMP::UsageException if invalid
  static void validate_method(std::string method) {
    IMP_ALWAYS_CHECK(get_allowed_methods().find(method)
                    != get_allowed_methods().end(),
                     "Invalid filtering method",
                     IMP::UsageException);
  }

public:
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_method_key(), pi)
           && m->get_has_attribute(get_threshold_key(), pi)
           && m->get_has_attribute(get_frames_key(), pi);
  }

  //! Set the filtering method
  //! \throw IMP::UsageException if not a valid method for filtering
  void set_method(std::string method) const {
    validate_method(method);
    return get_model()->set_attribute(get_method_key(), get_particle_index(),
                                      method);
  }

  //! \return the filtering method
  std::string get_method() const {
    return get_model()->get_attribute(get_method_key(), get_particle_index());
  }

  //! Set the number of frames
  void set_number_of_frames(int frames) const {
    return get_model()->set_attribute(get_frames_key(), get_particle_index(),
                                      frames);
  }

  //! \return the number of frames
  int get_number_of_frames() const {
    return get_model()->get_attribute(get_frames_key(), get_particle_index());
  }

  //! Set the score threshold
  void set_threshold(double threshold) const {
    return get_model()->set_attribute(get_threshold_key(), get_particle_index(),
                                      threshold);
  }

  //! \return the threshold
  double get_threshold() const {
    return get_model()->get_attribute(get_threshold_key(),
                                      get_particle_index());
  }

  IMP_DECORATOR_METHODS(FilterProvenance, Provenance);
  IMP_DECORATOR_SETUP_3(FilterProvenance, std::string, method,
                        double, threshold, int, frames);
  IMP_DECORATOR_SETUP_1(FilterProvenance, FilterProvenance, o);
};

//! Track creation of a system fragment from clustering.
/** Part of the system (usually the top of a Hierarchy) tagged with this
    decorator is understood to be a single frame inside a cluster of
    specified size. The rest of the cluster members are generally stored
    in a file (e.g. an RMF file).
  */
class IMPCOREEXPORT ClusterProvenance : public Provenance {
  static void do_setup_particle(Model *m, ParticleIndex pi, int members,
                                double precision=0., std::string density="") {
    Provenance::setup_particle(m, pi);
    m->add_attribute(get_members_key(), pi, members);
    m->add_attribute(get_precision_key(), pi, precision);
    if (density.empty()) {
      m->add_attribute(get_density_key(), pi, density);
    } else {
      m->add_attribute(get_density_key(), pi, get_absolute_path(density));
    }
  }

  static void do_setup_particle(Model *m, ParticleIndex pi,
                                ClusterProvenance o) {
    do_setup_particle(m, pi, o.get_number_of_members()), o.get_precision(),
                      o.get_density();
  }

  static IntKey get_members_key();
  static FloatKey get_precision_key();
  static StringKey get_density_key();

public:
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_members_key(), pi)
           && m->get_has_attribute(get_precision_key(), pi)
           && m->get_has_attribute(get_density_key(), pi);
  }

  //! Set the number of cluster members
  void set_number_of_members(int members) const {
    return get_model()->set_attribute(get_members_key(), get_particle_index(),
                                      members);
  }

  //! \return the number of cluster members
  int get_number_of_members() const {
    return get_model()->get_attribute(get_members_key(), get_particle_index());
  }

  //! Set the cluster precision
  void set_precision(double precision) const {
    return get_model()->set_attribute(get_precision_key(), get_particle_index(),
                                      precision);
  }

  //! \return the cluster precision
  double get_precision() const {
    return get_model()->get_attribute(get_precision_key(),
                                      get_particle_index());
  }

  //! Set the path to the localization probability density for this cluster
  /** Typically, this is used to point to an MRC file, but can be empty if
      no such density is available. The path can be relative or absolute.
      Internally, an absolute path will be stored (although generally it will
      be converted to a relative path when storing in a file, such as RMF).
    */
  void set_density(std::string density) const {
    if (!density.empty()) {
      density = get_absolute_path(density);
    }
    return get_model()->set_attribute(get_density_key(), get_particle_index(),
                                      density);
  }

  //! \return the localization probability density filename, as an absolute path
  std::string get_density() const {
    return get_model()->get_attribute(get_density_key(), get_particle_index());
  }

  IMP_DECORATOR_METHODS(ClusterProvenance, Provenance);
  IMP_DECORATOR_SETUP_1(ClusterProvenance, int, members);
  IMP_DECORATOR_SETUP_2(ClusterProvenance, int, members, double, precision);
  IMP_DECORATOR_SETUP_3(ClusterProvenance, int, members, double, precision,
                        std::string, density);
  IMP_DECORATOR_SETUP_1(ClusterProvenance, ClusterProvenance, o);
};

//! Track creation of a system fragment from running a script.
/** Part of the system (usually the top of a Hierarchy) tagged with this
    decorator is understood to have been generated by running a script
    (usually a Python script).
  */
class IMPCOREEXPORT ScriptProvenance : public Provenance {
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                std::string filename) {
    Provenance::setup_particle(m, pi);
    IMP_USAGE_CHECK(!filename.empty(), "The filename cannot be empty.");
    m->add_attribute(get_filename_key(), pi, get_absolute_path(filename));
  }

  static void do_setup_particle(Model *m, ParticleIndex pi,
                                ScriptProvenance o) {
    do_setup_particle(m, pi, o.get_filename());
  }

  static StringKey get_filename_key();

public:
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_filename_key(), pi);
  }

  //! Set the filename
  /** The path can be relative or absolute. Internally, an absolute
      path will be stored (although generally it will be converted to
      a relative path when storing in a file, such as RMF).
    */
  void set_filename(std::string filename) const {
    IMP_USAGE_CHECK(!filename.empty(), "The filename cannot be empty");
    return get_model()->set_attribute(get_filename_key(), get_particle_index(),
                                      get_absolute_path(filename));
  }

  //! \return the filename, as an absolute path
  std::string get_filename() const {
    return get_model()->get_attribute(get_filename_key(), get_particle_index());
  }

  IMP_DECORATOR_METHODS(ScriptProvenance, Provenance);
  IMP_DECORATOR_SETUP_1(ScriptProvenance, std::string, filename);
  IMP_DECORATOR_SETUP_1(ScriptProvenance, ScriptProvenance, o);
};

//! Track creation of a system fragment from running some software.
/** Part of the system (usually the top of a Hierarchy) tagged with this
    decorator is understood to have been generated by running the given
    software (e.g. IMP itself, or an extension module).

    \note While obviously every model has to be generated by IMP, it is
          useful to track the version of IMP used, as during a long protocol
	  it's possible (although not ideal!) for different IMP versions
	  to be used for different steps.
  */
class IMPCOREEXPORT SoftwareProvenance : public Provenance {
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                std::string name,
                                std::string version,
                                std::string location) {
    Provenance::setup_particle(m, pi);
    m->add_attribute(get_name_key(), pi, name);
    m->add_attribute(get_version_key(), pi, version);
    m->add_attribute(get_location_key(), pi, location);
  }

  static void do_setup_particle(Model *m, ParticleIndex pi,
                                SoftwareProvenance o) {
    do_setup_particle(m, pi, o.get_software_name(), o.get_version(),
                      o.get_location());
  }

  static StringKey get_name_key();
  static StringKey get_version_key();
  static StringKey get_location_key();

public:
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_name_key(), pi)
           && m->get_has_attribute(get_version_key(), pi)
           && m->get_has_attribute(get_location_key(), pi);
  }

  //! Set the name
  void set_software_name(std::string name) const {
    return get_model()->set_attribute(get_name_key(),
                                      get_particle_index(), name);
  }

  //! \return the name
  std::string get_software_name() const {
    return get_model()->get_attribute(get_name_key(), get_particle_index());
  }

  //! Set the version
  void set_version(std::string version) const {
    return get_model()->set_attribute(get_version_key(),
                                      get_particle_index(), version);
  }

  //! \return the version
  std::string get_version() const {
    return get_model()->get_attribute(get_version_key(),
                                      get_particle_index());
  }

  //! Set the location
  void set_location(std::string location) const {
    return get_model()->set_attribute(get_location_key(),
                                      get_particle_index(), location);
  }

  //! \return the location
  std::string get_location() const {
    return get_model()->get_attribute(get_location_key(),
                                      get_particle_index());
  }

  IMP_DECORATOR_METHODS(SoftwareProvenance, Provenance);
  IMP_DECORATOR_SETUP_3(SoftwareProvenance, std::string, name,
                        std::string, version, std::string, location);
  IMP_DECORATOR_SETUP_1(SoftwareProvenance, SoftwareProvenance, o);
};

//! Tag part of the system to track how it was created.
class IMPCOREEXPORT Provenanced : public Decorator {
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

//! Add provenance to part of the model.
IMPCOREEXPORT void add_provenance(Model *m, ParticleIndex pi,
                                  Provenance p);

//! Clone provenance (including previous provenance)
IMPCOREEXPORT Provenance create_clone(Provenance p);

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_PROVENANCE_H */
