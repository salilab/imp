/**
 *  \file IMP/core/provenance.h
 *  \brief Classes to track how the model was created.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_PROVENANCE_H
#define IMPCORE_PROVENANCE_H

#include <IMP/core/core_config.h>

#include <IMP/base_types.h>
#include <IMP/Object.h>
#include <IMP/object_macros.h>
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>
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
class IMPCOREEXPORT StructureProvenance : public Provenance {
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                std::string filename,
                                std::string chain_id) {
    Provenance::setup_particle(m, pi);
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

//! Track creation of a system fragment from sampling.
/** Part of the system (usually the top of a Hierarchy) tagged with this
    decorator is understood to be a single frame from an ensemble of
    multiple frames generated with some sampling method (e.g. Monte Carlo).
    Additionally, the number of iterations of the sampler used to generate
    each frame can be stored, if known and applicable.
    The rest of the frames are generally stored in a file (e.g. an RMF file).
  */
class IMPCOREEXPORT SampleProvenance : public Provenance {
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                std::string method, int frames,
                                int iterations) {
    validate_method(method);
    Provenance::setup_particle(m, pi);
    m->add_attribute(get_method_key(), pi, method);
    m->add_attribute(get_frames_key(), pi, frames);
    m->add_attribute(get_iterations_key(), pi, iterations);
  }

  static StringKey get_method_key();
  static IntKey get_frames_key();
  static IntKey get_iterations_key();

  static std::set<std::string>& get_allowed_methods();

  static void validate_method(std::string method) {
    IMP_USAGE_CHECK(get_allowed_methods().find(method)
                                 != get_allowed_methods().end(),
                    "Invalid sampling method");
  }

public:
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_method_key(), pi)
           && m->get_has_attribute(get_iterations_key(), pi)
           && m->get_has_attribute(get_frames_key(), pi);
  }

  //! Set the sampling method
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

  IMP_DECORATOR_METHODS(SampleProvenance, Provenance);
  IMP_DECORATOR_SETUP_3(SampleProvenance, std::string, method, int, frames,
                        int, iterations);
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
};

//! Track creation of a system fragment by filtering.
/** Part of the system (usually the top of a Hierarchy) tagged with this
    decorator is understood to be a single frame within an ensemble that
    resulted from filtering a larger ensemble (the 'previous'
    provenance) by discarding models with scores above the threshold.
  */
class IMPCOREEXPORT FilterProvenance : public Provenance {
  static void do_setup_particle(Model *m, ParticleIndex pi, double threshold,
                                int frames) {
    Provenance::setup_particle(m, pi);
    m->add_attribute(get_threshold_key(), pi, threshold);
    m->add_attribute(get_frames_key(), pi, frames);
  }

  static FloatKey get_threshold_key();
  static IntKey get_frames_key();

public:
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_threshold_key(), pi)
           && m->get_has_attribute(get_frames_key(), pi);
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
  IMP_DECORATOR_SETUP_2(FilterProvenance, double, threshold, int, frames);
};

//! Track creation of a system fragment from clustering.
/** Part of the system (usually the top of a Hierarchy) tagged with this
    decorator is understood to be a single frame inside a cluster of
    specified size. The rest of the cluster members are generally stored
    in a file (e.g. an RMF file).
  */
class IMPCOREEXPORT ClusterProvenance : public Provenance {
  static void do_setup_particle(Model *m, ParticleIndex pi, int members) {
    Provenance::setup_particle(m, pi);
    m->add_attribute(get_members_key(), pi, members);
  }

  static IntKey get_members_key();

public:
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_members_key(), pi);
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

  IMP_DECORATOR_METHODS(ClusterProvenance, Provenance);
  IMP_DECORATOR_SETUP_1(ClusterProvenance, int, members);
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

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_PROVENANCE_H */
