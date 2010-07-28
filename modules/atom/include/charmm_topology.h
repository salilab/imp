/**
 * \file charmm_topology.h \brief Classes for handling CHARMM-style topology.
 *
 * Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_CHARMM_TOPOLOGY_H
#define IMPATOM_CHARMM_TOPOLOGY_H

#include "IMP/Object.h"
#include "IMP/container_macros.h"
#include "Hierarchy.h"
#include "Atom.h"
#include "atom_config.h"

#include <string>
#include <vector>

IMPATOM_BEGIN_NAMESPACE

//! A single atom in a CHARMM topology.
/** Each atom has a name (unique to the residue), a CHARMM type (used to
    look up parameters such as radii and bond lengths in the parameter file)
    and an electrostatic charge.
    \see CHARMMAtom
 */
class CHARMMAtomTopology {
  std::string name_;
  std::string charmm_type_;
  double charge_;
public:
  CHARMMAtomTopology(std::string name) : name_(name) {};

  CHARMMAtomTopology(std::string name, const CHARMMAtomTopology &other)
                   : name_(name), charmm_type_(other.charmm_type_),
                     charge_(other.charge_) {};

  std::string get_name() const { return name_; }
  std::string get_charmm_type() const { return charmm_type_; }
  double get_charge() const { return charge_; }
  void set_charmm_type(std::string charmm_type) { charmm_type_ = charmm_type; }
  void set_charge(double charge) { charge_ = charge; }
};

class CHARMMResidueTopology;

//! The end of a bond, angle, dihedral, or improper.
/** An endpoint is an atom in a residue; bonds have two endpoints,
    angles three, and dihedrals and impropers, four.

    If residue_ is not NULL, the endpoint is the named atom in the
    pointed-to residue. (This is used for bonds that can span multiple
    residues, perhaps in different chains, and is typically only created
    by applying a two-residue patch such as DISU or LINK).

    If residue_ is NULL, the endpoint is the named atom in the "current"
    residue, unless the atom name has a +, -, 1: or 2: prefix.
    Atoms prefixed with + are found in the next residue in the chain.
    Atoms prefixed with - are found in the previous residue in the chain.
    1: or 2: prefixes are used by two-residue patches to refer to atoms in
    each residue patched by the patch.
 */
class CHARMMBondEndpoint {
  std::string atom_name_;
  CHARMMResidueTopology *residue_;
public:
  CHARMMBondEndpoint(std::string atom_name,
                     CHARMMResidueTopology *residue=NULL)
                    : atom_name_(atom_name), residue_(residue) {}

  std::string get_atom_name() const { return atom_name_; }

  //! Map the endpoint to an Atom particle.
  Atom get_atom(const CHARMMResidueTopology *current_residue,
                const CHARMMResidueTopology *previous_residue,
                const CHARMMResidueTopology *next_residue,
                const std::map<const CHARMMResidueTopology *,
                               Hierarchy> &resmap) const {
    if (residue_) {
      return IMP::atom::get_atom(resmap.find(residue_)->second.get_as_residue(),
                                 AtomType(atom_name_));
    } else if (atom_name_[0] == '+') {
      if (next_residue) {
        return IMP::atom::get_atom(resmap.find(next_residue)->second.
                                                         get_as_residue(),
                                   AtomType(atom_name_.substr(1)));
      } else {
        return Atom();
      }
    } else if (atom_name_[0] == '-') {
      if (previous_residue) {
        return IMP::atom::get_atom(resmap.find(previous_residue)->second.
                                                           get_as_residue(),
                                   AtomType(atom_name_.substr(1)));
      } else {
        return Atom();
      }
    } else {
      return IMP::atom::get_atom(resmap.find(current_residue)->second.
                                                          get_as_residue(),
                                 AtomType(atom_name_));
    }
  }
};

//! A bond, angle, dihedral or improper between some number of endpoints.
template <unsigned int D>
class CHARMMBond
{
  std::vector<CHARMMBondEndpoint> endpoints_;
public:
  CHARMMBond(const Strings &atoms) {
    IMP_INTERNAL_CHECK(atoms.size() == D, "wrong number of bond endpoints");
    for (std::vector<std::string>::const_iterator it = atoms.begin();
         it != atoms.end(); ++it) {
      endpoints_.push_back(CHARMMBondEndpoint(*it));
    }
  }

  CHARMMBond(std::vector<CHARMMBondEndpoint> endpoints)
     : endpoints_(endpoints) {
    IMP_INTERNAL_CHECK(endpoints.size() == D, "wrong number of bond endpoints");
  }

  const CHARMMBondEndpoint & get_endpoint(unsigned int i) const {
    return endpoints_[i];
  }

  //! \return true if the bond contains the named atom.
  bool contains_atom(std::string name) const {
    for (std::vector<CHARMMBondEndpoint>::const_iterator
         it = endpoints_.begin(); it != endpoints_.end(); ++it) {
      if (it->get_atom_name() == name) {
        return true;
      }
    }
    return false;
  }

  //! Map the bond to a list of Atom particles.
  Atoms get_atoms(const CHARMMResidueTopology *current_residue,
                  const CHARMMResidueTopology *previous_residue,
                  const CHARMMResidueTopology *next_residue,
                  const std::map<const CHARMMResidueTopology *,
                                 Hierarchy> &resmap) const {
    Atoms as;
    for (std::vector<CHARMMBondEndpoint>::const_iterator
         it = endpoints_.begin(); it != endpoints_.end(); ++it) {
      Atom a = it->get_atom(current_residue, previous_residue,
                            next_residue, resmap);
      if (a) {
        as.push_back(a);
      } else {
        return Atoms();
      }
    }
    return as;
  }
};

//! Base class for all CHARMM residue-based topology
class IMPATOMEXPORT CHARMMResidueTopologyBase {
  std::string type_;
protected:
  std::vector<CHARMMAtomTopology> atoms_;
  std::vector<CHARMMBond<2> > bonds_;
  std::vector<CHARMMBond<3> > angles_;
  std::vector<CHARMMBond<4> > dihedrals_;
  std::vector<CHARMMBond<4> > impropers_;

  CHARMMResidueTopologyBase(std::string type) : type_(type) {}
public:
  std::string get_type() const { return type_; }

  unsigned int get_number_of_atoms() const { return atoms_.size(); }
  const CHARMMAtomTopology &get_atom(unsigned int i) const { return atoms_[i]; }

  void add_atom(const CHARMMAtomTopology &atom);
  CHARMMAtomTopology &get_atom(std::string name);
  const CHARMMAtomTopology &get_atom(AtomType type) const {
    return get_atom(type.get_string());
  }

#ifndef SWIG
  const CHARMMAtomTopology &get_atom(std::string name) const;
#endif

  unsigned int get_number_of_bonds() const { return bonds_.size(); }
  void add_bond(const CHARMMBond<2> &bond) {
    bonds_.push_back(bond);
  }
  CHARMMBond<2> &get_bond(unsigned int index) { return bonds_[index]; }

  unsigned int get_number_of_angles() const { return angles_.size(); }
  void add_angle(const CHARMMBond<3> &bond) {
    angles_.push_back(bond);
  }
  CHARMMBond<3> &get_angle(unsigned int index) { return angles_[index]; }

  unsigned int get_number_of_dihedrals() const { return dihedrals_.size(); }
  void add_dihedral(const CHARMMBond<4> &bond) {
    dihedrals_.push_back(bond);
  }
  CHARMMBond<4> &get_dihedral(unsigned int index) { return dihedrals_[index]; }

  unsigned int get_number_of_impropers() const { return impropers_.size(); }
  void add_improper(const CHARMMBond<4> &bond) {
    impropers_.push_back(bond);
  }
  CHARMMBond<4> &get_improper(unsigned int index) { return impropers_[index]; }

#ifndef SWIG
  const CHARMMBond<2> &get_bond(unsigned int index) const {
    return bonds_[index];
  }
  const CHARMMBond<3> &get_angle(unsigned int index) const {
    return angles_[index];
  }
  const CHARMMBond<4> &get_dihedral(unsigned int index) const {
    return dihedrals_[index];
  }
  const CHARMMBond<4> &get_improper(unsigned int index) const {
    return impropers_[index];
  }
#endif
};

//! The ideal topology of a single residue
/** These residue topologies can be constructed manually (by adding
    CHARMMAtomTopology objects with add_atom()). However, they are more
    commonly populated automatically by reading a CHARMM topology file
    (CHARMMParameters::get_residue_topology()).
 */
class IMPATOMEXPORT CHARMMIdealResidueTopology
    : public CHARMMResidueTopologyBase {
  std::string default_first_patch_, default_last_patch_;
public:
  CHARMMIdealResidueTopology(ResidueType type)
      : CHARMMResidueTopologyBase(type.get_string()) {}

  //! Delete the named atom
  /** Any bonds/angles that involve this atom are also deleted.
   */
  void delete_atom(std::string name);

  void set_default_first_patch(std::string patch) {
    default_first_patch_ = patch;
  }
  void set_default_last_patch(std::string patch) {
    default_last_patch_ = patch;
  }
  std::string get_default_first_patch() const { return default_first_patch_; }
  std::string get_default_last_patch() const { return default_last_patch_; }
};

class CHARMMResidueTopology;

//! A CHARMM patch residue.
/** Patch residues are similar to regular residues, except that they are
    used to modify an existing residue. Any atoms they contain replace or
    add to those in the residue; they can also remove atoms.
    Atom names are as for regular residues, except for patches that affect
    two residues (e.g. DISU, LINK) in which each atom contains a 1: or 2:
    prefix to identify the residue which will be patched.
 */
class IMPATOMEXPORT CHARMMPatch : public CHARMMResidueTopologyBase {
  std::vector<std::string> deleted_atoms_;
public:
  CHARMMPatch(std::string type) : CHARMMResidueTopologyBase(type) {}

  void add_deleted_atom(std::string name) { deleted_atoms_.push_back(name); }

  //! Apply the patch to the residue, modifying its topology accordingly.
  /** \note Most CHARMM patches are designed to be applied in isolation;
            it is usually an error to try to apply two different patches
            to the same residue. Thus, by default \imp prohibits this.
            To allow an already-patched residue to be re-patched, first
            call CHARMMResidueTopology::set_patched(false).
   */
  void apply(CHARMMResidueTopology &res) const;

  //! Apply the patch to the given pair of residues.
  /** This can only be used for special two-residue patches, such as
      DISU or LINK. In a two-residue patch, each atom has a 1: or 2: prefix
      to identify the residue it refers to.

      \throws ValueException if the patch is not a two-residue patch.
   */
  void apply(CHARMMResidueTopology &res1, CHARMMResidueTopology &res2) const;
};

//! The topology of a single residue in a model.
/** Each CHARMMResidueTopology object can represent an 'unknown' residue
    (containing no atoms or bonds) or is a copy of an existing
    CHARMMIdealResidueTopology. Optionally, patches can be applied to
    residues (see CHARMMPatch::apply()) to add N- or C-termini, disulfide
    bridges, sidechain modifications, etc.
 */
class IMPATOMEXPORT CHARMMResidueTopology
     : public CHARMMIdealResidueTopology, public Object {
  bool patched_;
public:

  //! Create an empty topology, containing no atoms or bonds.
  CHARMMResidueTopology(ResidueType type)
    : CHARMMIdealResidueTopology(type), patched_(false) {}

  //! Construct residue topology as a copy of an existing topology.
  CHARMMResidueTopology(const CHARMMIdealResidueTopology &ideal)
    : CHARMMIdealResidueTopology(ideal), patched_(false) {}

  bool get_patched() const { return patched_; }
  void set_patched(bool patched) { patched_ = patched; }

  IMP_OBJECT(CHARMMResidueTopology);
};

IMP_OBJECTS(CHARMMResidueTopology,CHARMMResidueTopologys);

class CHARMMParameters;

//! The topology of a single CHARMM segment in a model.
/** CHARMM segments typically correspond to IMP::atom::Chain particles.
 */
class IMPATOMEXPORT CHARMMSegmentTopology : public Object {
  IMP_LIST(public, CHARMMResidueTopology, residue, CHARMMResidueTopology*,
           CHARMMResidueTopologys);

  IMP_OBJECT(CHARMMSegmentTopology);
public:
  //! Apply patches to the first and last residue in the segment.
  /** Default patches are defined for each residue type in the topology
      file. For example, segments containing amino acids will by default
      apply the CTER and NTER patches to the C and N termini, respectively.
   */
  void apply_default_patches(const CHARMMParameters *ff);
};

IMP_OBJECTS(CHARMMSegmentTopology,CHARMMSegmentTopologys);

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
           CHARMMSegmentTopologys);

  IMP_OBJECT(CHARMMTopology);
private:
  WarningContext warn_context_;
  typedef std::map<const CHARMMResidueTopology *, Hierarchy> ResMap;

  void map_residue_topology_to_hierarchy(Hierarchy hierarchy,
                                         ResMap &resmap) const;
public:
  //! Call CHARMMSegmentTopology::apply_default_patches() for all segments.
  void apply_default_patches(const CHARMMParameters *ff) {
    for (unsigned int i = 0; i < get_number_of_segments(); ++i) {
      get_segment(i)->apply_default_patches(ff);
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
      the existing bond graph (see CHARMMParameters::generate_angles() and
      CHARMMParameters::generate_dihedrals()).

      The list of newly-created Bond particles can be passed to a
      StereochemistryPairFilter to exclude bonded particles from nonbonded
      interactions, or to a BondSingletonScore to score each bond.

      \return a list of the generated Bond decorators.
   */
  Particles add_bonds(Hierarchy hierarchy, const CHARMMParameters *ff) const;

  //! Add impropers to the given Hierarchy using this topology, and return them.
  /** The primary sequence of the Hierarchy must match that of the topology.

      The list of newly-created Dihedral particles can be passed to a
      ImproperSingletonScore to score each improper dihedral.

      \return a list of the generated Dihedral decorators.

      \see add_bonds().
   */
  Particles add_impropers(Hierarchy hierarchy,
                          const CHARMMParameters *ff) const;
};

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_CHARMM_TOPOLOGY_H */
