/**
 * \file charmm_segment_topology.h
 * \brief Classes for handling CHARMM-style topology of segments.
 *
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_CHARMM_SEGMENT_TOPOLOGY_H
#define IMPATOM_CHARMM_SEGMENT_TOPOLOGY_H

#include "IMP/Object.h"
#include "Hierarchy.h"
#include "atom_config.h"
#include "charmm_topology.h"
#include "CHARMMParameters.h"

IMPATOM_BEGIN_NAMESPACE

//! The topology of a single CHARMM segment in a model.
/** CHARMM segments typically correspond to IMP::atom::Chain particles.
 */
class IMPATOMEXPORT CHARMMSegmentTopology : public Object {
  IMP_LIST(public, CHARMMResidueTopology, residue, CHARMMResidueTopology*,
           CHARMMResidueTopologies);

  IMP_OBJECT(CHARMMSegmentTopology);
public:
  CHARMMSegmentTopology(std::string name = "CHARMM segment topology %1%")
                       : Object(name) {}

  //! Apply patches to the first and last residue in the segment.
  /** Default patches are defined for each residue type in the topology
      file. For example, segments containing amino acids will by default
      apply the CTER and NTER patches to the C and N termini, respectively.
   */
  void apply_default_patches(const CHARMMParameters *ff);
};

IMP_OBJECTS(CHARMMSegmentTopology,CHARMMSegmentTopologies);

//! The topology of a complete CHARMM model.
/** This defines all of the segments (chains) in the model as
    CHARMMSegmentTopology objects, which in turn define the list of residues,
    the atoms in each residue, and their types and the connectivity
    between them.

    A CHARMMTopology object can be created manually, in which case add_segment()
    can be called to add individual CHARMMSegmentTopology objects. In this way
    a new topology can be created, e.g. from protein primary sequence.

    Alternatively, given an existing Hierarchy, e.g. as returned from
    read_pdb(), CHARMMParameters::create_topology() can be called to generate
    a new topology that corresponds to the primary sequence of the Hierarchy.

    A new topology can be patched (apply_default_patches() or
    CHARMMPatch::apply()) to modify the simple chain of residues to
    account for modified residues, C- or N-termini special cases, disulfide
    bridges, etc.

    Once a topology is created, it can be used to generate new particles
    which conform to that topology (create_hierarchy()), or to add
    topology information to an existing Hierarchy (e.g. add_atom_types(),
    add_bonds(), add_charges()).
 */
class IMPATOMEXPORT CHARMMTopology : public Object {
  IMP_LIST(public, CHARMMSegmentTopology, segment, CHARMMSegmentTopology*,
           CHARMMSegmentTopologies);

  IMP_OBJECT(CHARMMTopology);
private:
  Pointer<const CHARMMParameters> force_field_;
  WarningContext warn_context_;
  typedef std::map<const CHARMMResidueTopology *, Hierarchy> ResMap;

  void map_residue_topology_to_hierarchy(Hierarchy hierarchy,
                                         ResMap &resmap) const;
public:
  CHARMMTopology(const CHARMMParameters *force_field,
                 std::string name = "CHARMM topology %1%")
                 : Object(name), force_field_(force_field) {
    set_was_used(true);
  }

  //! Call CHARMMSegmentTopology::apply_default_patches() for all segments.
  void apply_default_patches() {
    for (unsigned int i = 0; i < get_number_of_segments(); ++i) {
      get_segment(i)->apply_default_patches(force_field_);
    }
  }

  //! Create a new Hierarchy in the given model using this topology.
  /** The hierarchy contains chains, residues and atoms as defined in the
      topology. Note, however, that none of the generated atoms is given
      coordinates.
   */
  Hierarchy create_hierarchy(Model *model) const;

  //! Add CHARMM atom types to the given Hierarchy using this topology.
  /** The primary sequence of the Hierarchy must match that of the topology.
      \see CHARMMAtom.
   */
  void add_atom_types(Hierarchy hierarchy) const;

  //! Add any missing atoms to the given Hierarchy using this topology.
  /** Missing atoms are defined as those present in the topology but not
      in the hierarchy.
      Newly-added atoms are assigned CHARMM types, but no coordinates.
      The primary sequence of the Hierarchy must match that of the topology.
      \see CHARMMAtom, remove_charmm_untyped_atoms.
   */
  void add_missing_atoms(Hierarchy hierarchy) const;


  //! Add CHARMM charges to the given Hierarchy using this topology.
  /** The primary sequence of the Hierarchy must match that of the topology.
      \see Charged.
   */
  void add_charges(Hierarchy hierarchy) const;

  //! Add bonds to the given Hierarchy using this topology, and return them.
  /** The primary sequence of the Hierarchy must match that of the topology.
      Parameters for the bonds (ideal bond length, force constant) are
      extracted from the CHARMM parameter file, using the types of each atom
      (add_atom_types() must be called first, or the particles otherwise
      manually typed using CHARMMAtom::set_charmm_type()).

      If no parameters are defined for a given bond, the bond is created
      with zero stiffness, such that the bond can still be excluded from
      nonbonded interactions but BondSingletonScore will not score it.

      Note that typically CHARMM defines bonds and impropers
      (see add_impropers()) but angles and dihedrals are auto-generated from
      the existing bond graph (see CHARMMParameters::create_angles() and
      CHARMMParameters::create_dihedrals()).

      The list of newly-created Bond particles can be passed to a
      StereochemistryPairFilter to exclude bonded particles from nonbonded
      interactions, or to a BondSingletonScore to score each bond.

      \return a list of the generated Bond decorators.
   */
  Particles add_bonds(Hierarchy hierarchy) const;

  //! Add impropers to the given Hierarchy using this topology, and return them.
  /** The primary sequence of the Hierarchy must match that of the topology.

      The list of newly-created Dihedral particles can be passed to a
      ImproperSingletonScore to score each improper dihedral.

      \return a list of the generated Dihedral decorators.

      \see add_bonds().
   */
  Particles add_impropers(Hierarchy hierarchy) const;
};

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_CHARMM_SEGMENT_TOPOLOGY_H */
