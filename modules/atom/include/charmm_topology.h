/**
 * \file IMP/atom/charmm_topology.h
 * \brief Classes for handling CHARMM-style topology.
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_CHARMM_TOPOLOGY_H
#define IMPATOM_CHARMM_TOPOLOGY_H

#include "IMP/base/Object.h"
#include "Hierarchy.h"
#include "Atom.h"
#include <IMP/atom/atom_config.h>

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
  CHARMMAtomTopology(std::string name) : name_(name) {}

  CHARMMAtomTopology(std::string name, const CHARMMAtomTopology &other)
      : name_(name), charmm_type_(other.charmm_type_), charge_(other.charge_) {}

  std::string get_name() const { return name_; }
  std::string get_charmm_type() const { return charmm_type_; }
  double get_charge() const { return charge_; }
  void set_charmm_type(std::string charmm_type) { charmm_type_ = charmm_type; }
  void set_charge(double charge) { charge_ = charge; }
  IMP_SHOWABLE_INLINE(CHARMMAtomTopology, {
    out << "name: " << name_ << "; CHARMM type: " << charmm_type_
        << "; charge: " << charge_;
  });
};

IMP_VALUES(CHARMMAtomTopology, CHARMMAtomTopologies);

class CHARMMResidueTopology;

//! The end of a bond, angle, dihedral, improper, or internal coordinate.
/** An endpoint is an atom in a residue; bonds have two endpoints,
    angles three, and dihedrals, impropers and internal coordinates, four.

    If residue_ is not nullptr, the endpoint is the named atom in the
    pointed-to residue. (This is used for bonds that can span multiple
    residues, perhaps in different chains, and is typically only created
    by applying a two-residue patch such as DISU or LINK).

    If residue_ is nullptr, the endpoint is the named atom in the "current"
    residue, unless the atom name has a +, -, 1: or 2: prefix.
    Atoms prefixed with + are found in the next residue in the chain.
    Atoms prefixed with - are found in the previous residue in the chain.
    1: or 2: prefixes are used by two-residue patches to refer to atoms in
    each residue patched by the patch.
 */
class IMPATOMEXPORT CHARMMBondEndpoint {
  std::string atom_name_;
  base::Pointer<base::Object> residue_;

 public:
  CHARMMBondEndpoint(std::string atom_name,
                     CHARMMResidueTopology *residue = nullptr);

  std::string get_atom_name() const { return atom_name_; }

  //! Map the endpoint to an Atom particle.
  Atom get_atom(
      const CHARMMResidueTopology *current_residue,
      const CHARMMResidueTopology *previous_residue,
      const CHARMMResidueTopology *next_residue,
      const std::map<const CHARMMResidueTopology *, Hierarchy> &resmap) const;

  IMP_SHOWABLE_INLINE(CHARMMBondEndpoint, {
    out << atom_name_;
  });
};

IMP_VALUES(CHARMMBondEndpoint, CHARMMBondEndpoints);

//! A connection (bond, angle, dihedral) between some number of endpoints.
template <unsigned int D>
class CHARMMConnection {
 protected:
  base::Vector<CHARMMBondEndpoint> endpoints_;

 public:
  CHARMMConnection(const IMP::Strings &atoms) {
    IMP_INTERNAL_CHECK(atoms.size() == D, "wrong number of bond endpoints");
    for (Strings::const_iterator it = atoms.begin(); it != atoms.end(); ++it) {
      endpoints_.push_back(CHARMMBondEndpoint(*it));
    }
  }

#ifndef SWIG
  CHARMMConnection(base::Vector<CHARMMBondEndpoint> endpoints)
      : endpoints_(endpoints) {
    IMP_INTERNAL_CHECK(endpoints.size() == D, "wrong number of bond endpoints");
  }
#endif

  const IMP::atom::CHARMMBondEndpoint &get_endpoint(unsigned int i) const {
    return endpoints_[i];
  }

  //! \return true if the bond contains the named atom.
  bool get_contains_atom(std::string name) const {
    for (base::Vector<CHARMMBondEndpoint>::const_iterator it =
             endpoints_.begin();
         it != endpoints_.end(); ++it) {
      if (it->get_atom_name() == name) {
        return true;
      }
    }
    return false;
  }

#ifndef SWIG
  //! Map the bond to a list of Atom particles.
  Atoms get_atoms(
      const CHARMMResidueTopology *current_residue,
      const CHARMMResidueTopology *previous_residue,
      const CHARMMResidueTopology *next_residue,
      const std::map<const CHARMMResidueTopology *, Hierarchy> &resmap) const {
    Atoms as;
    for (base::Vector<CHARMMBondEndpoint>::const_iterator it =
             endpoints_.begin();
         it != endpoints_.end(); ++it) {
      Atom a =
          it->get_atom(current_residue, previous_residue, next_residue, resmap);
      if (a) {
        as.push_back(a);
      } else {
        return Atoms();
      }
    }
    return as;
  }
#endif
  IMP_SHOWABLE_INLINE(CHARMMConnection, {
    for (base::Vector<CHARMMBondEndpoint>::const_iterator it =
             endpoints_.begin();
         it != endpoints_.end(); ++it) {
      if (it != endpoints_.begin()) {
        out << "-";
      }
      out << it->get_atom_name();
    }
  });
};

typedef CHARMMConnection<2> CHARMMBond;
typedef CHARMMConnection<3> CHARMMAngle;
typedef CHARMMConnection<4> CHARMMDihedral;

IMP_VALUES(CHARMMBond, CHARMMBonds);

IMP_VALUES(CHARMMAngle, CHARMMAngles);

IMP_VALUES(CHARMMDihedral, CHARMMDihedrals);

#ifdef SWIG
// Ugly, but SWIG needs a template instantiation before it is used as
// a base class
%template(CHARMMDihedral) CHARMMConnection<4>;
#endif

//! A geometric relationship between four atoms.
/** The atoms (denoted i,j,k,l here) are uniquely positioned in 3D space
    relative to each other by means of two distances, two angles, and
    a dihedral.

    A regular internal coordinate stores the distances between ij and
    kl respectively, and the angles between ijk and jkl.

    An improper internal coordinate stores the distances between ik and
    kl respectively, and the angles between ikj and jkl.

    In both cases the dihedral is the angle between the two planes formed
    by ijk and jkl.
 */
class CHARMMInternalCoordinate : public CHARMMConnection<4> {
  float first_distance_, second_distance_, first_angle_, second_angle_,
      dihedral_;
  bool improper_;

 public:
  CHARMMInternalCoordinate(const IMP::Strings &atoms, float first_distance,
                           float first_angle, float dihedral,
                           float second_angle, float second_distance,
                           bool improper)
      : CHARMMConnection<4>(atoms),
        first_distance_(first_distance),
        second_distance_(second_distance),
        first_angle_(first_angle),
        second_angle_(second_angle),
        dihedral_(dihedral),
        improper_(improper) {}
  CHARMMInternalCoordinate(const base::Vector<CHARMMBondEndpoint> endpoints,
                           float first_distance, float first_angle,
                           float dihedral, float second_angle,
                           float second_distance, bool improper)
      : CHARMMConnection<4>(endpoints),
        first_distance_(first_distance),
        second_distance_(second_distance),
        first_angle_(first_angle),
        second_angle_(second_angle),
        dihedral_(dihedral),
        improper_(improper) {}

  float get_first_distance() const { return first_distance_; }
  float get_second_distance() const { return second_distance_; }
  float get_first_angle() const { return first_angle_; }
  float get_second_angle() const { return second_angle_; }
  float get_dihedral() const { return dihedral_; }
  bool get_improper() const { return improper_; }

  IMP_SHOWABLE_INLINE(CHARMMInternalCoordinate, {
    CHARMMConnection<4>::show(out);
    out << "; distances: " << first_distance_ << ", " << second_distance_
        << "; angles: " << first_angle_ << ", " << second_angle_
        << "; dihedral: " << dihedral_;
    if (improper_) {
      out << "; improper";
    }
  });
};
IMP_VALUES(CHARMMInternalCoordinate, CHARMMInternalCoordinates);

//! Base class for all CHARMM residue-based topology
class IMPATOMEXPORT CHARMMResidueTopologyBase : public IMP::base::Object {
  std::string type_;

 protected:
  base::Vector<CHARMMAtomTopology> atoms_;
  CHARMMBonds bonds_;
  CHARMMAngles angles_;
  CHARMMDihedrals dihedrals_;
  CHARMMDihedrals impropers_;
  CHARMMInternalCoordinates internal_coordinates_;

  CHARMMResidueTopologyBase(std::string type)
      : Object("CharmmResidueTopoBase%1%"), type_(type) {
    set_name(std::string("CHARMM residue ") + type);
  }

 public:
  std::string get_type() const { return type_; }

  unsigned int get_number_of_atoms() const { return atoms_.size(); }
  const CHARMMAtomTopology &get_atom(unsigned int i) const { return atoms_[i]; }

  void add_atom(const CHARMMAtomTopology &atom);
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  CHARMMAtomTopology &get_atom(std::string name);
#endif
  const CHARMMAtomTopology &get_atom(AtomType type) const {
    return get_atom(type.get_string());
  }

  const CHARMMAtomTopology &get_atom(std::string name) const;

  unsigned int get_number_of_bonds() const { return bonds_.size(); }
  void add_bond(const CHARMMBond &bond) { bonds_.push_back(bond); }
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  // cannot return non const ref to swig
  CHARMMBond &get_bond(unsigned int index) { return bonds_[index]; }
#endif

  unsigned int get_number_of_angles() const { return angles_.size(); }
  void add_angle(const CHARMMAngle &bond) { angles_.push_back(bond); }
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  // cannot return non const ref to swig
  CHARMMAngle &get_angle(unsigned int index) { return angles_[index]; }
#endif
  unsigned int get_number_of_dihedrals() const { return dihedrals_.size(); }
  void add_dihedral(const CHARMMDihedral &bond) { dihedrals_.push_back(bond); }
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  // cannot return non const ref to swig
  CHARMMDihedral &get_dihedral(unsigned int index) { return dihedrals_[index]; }
#endif
  unsigned int get_number_of_impropers() const { return impropers_.size(); }
  void add_improper(const CHARMMDihedral &bond) { impropers_.push_back(bond); }
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  CHARMMDihedral &get_improper(unsigned int index) { return impropers_[index]; }
#endif

  unsigned int get_number_of_internal_coordinates() const {
    return internal_coordinates_.size();
  }
  void add_internal_coordinate(const CHARMMInternalCoordinate &ic) {
    internal_coordinates_.push_back(ic);
  }
  const CHARMMInternalCoordinate &get_internal_coordinate(
      unsigned int index) const {
    return internal_coordinates_[index];
  }

  const CHARMMBond &get_bond(unsigned int index) const { return bonds_[index]; }
  const CHARMMAngle &get_angle(unsigned int index) const {
    return angles_[index];
  }
  const CHARMMDihedral &get_dihedral(unsigned int index) const {
    return dihedrals_[index];
  }
  const CHARMMDihedral &get_improper(unsigned int index) const {
    return impropers_[index];
  }

  IMP_OBJECT_METHODS(CHARMMResidueTopologyBase);
};

IMP_OBJECTS(CHARMMResidueTopologyBase, CHARMMResidueTopologyBases);

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
  CHARMMIdealResidueTopology(std::string type)
      : CHARMMResidueTopologyBase(type) {}
  CHARMMIdealResidueTopology(ResidueType type)
      : CHARMMResidueTopologyBase(type.get_string()) {}

  //! Remove the named atom
  /** Any bonds/angles that involve this atom are also removed.
   */
  void remove_atom(std::string name);

  void set_default_first_patch(std::string patch) {
    default_first_patch_ = patch;
  }
  void set_default_last_patch(std::string patch) {
    default_last_patch_ = patch;
  }
  std::string get_default_first_patch() const { return default_first_patch_; }
  std::string get_default_last_patch() const { return default_last_patch_; }
  IMP_OBJECT_METHODS(CHARMMIdealResidueTopology);
};

IMP_OBJECTS(CHARMMIdealResidueTopology, CHARMMIdealResidueTopologies);

class CHARMMResidueTopology;

//! A CHARMM patch residue.
/** Patch residues are similar to regular residues, except that they are
    used to modify an existing residue. Any atoms they contain replace or
    add to those in the residue; they can also remove atoms.
    Atom names are as for regular residues, except for patches that affect
    two residues (e.g. DISU, LINK) in which each atom contains a 1 or 2
    prefix (plus an optional colon) to identify the residue which will be
    patched; for example, both 1:CA and 1CA identify the CA atom in the
    first residue.
 */
class IMPATOMEXPORT CHARMMPatch : public CHARMMResidueTopologyBase {
  base::Vector<std::string> deleted_atoms_;

 public:
  //! Construct a new, empty patch residue.
  /** To get an existing patch, use CHARMMParameters::get_patch() instead.
   */
  CHARMMPatch(std::string type) : CHARMMResidueTopologyBase(type) {
    set_name(std::string("CHARMM patching residue ") + type);
  }

  void add_removed_atom(std::string name) { deleted_atoms_.push_back(name); }

  unsigned int get_number_of_removed_atoms() const {
    return deleted_atoms_.size();
  }

  std::string get_removed_atom(unsigned int i) const {
    return deleted_atoms_[i];
  }

  //! Apply the patch to the residue, modifying its topology accordingly.
  /** \note Most CHARMM patches are designed to be applied in isolation;
            it is usually an error to try to apply two different patches
            to the same residue. Thus, by default \imp prohibits this.
            To allow an already-patched residue to be re-patched, first
            call CHARMMResidueTopology::set_patched(false).
   */
  void apply(CHARMMResidueTopology *res) const;

  //! Apply the patch to the given pair of residues.
  /** This can only be used for special two-residue patches, such as
      DISU or LINK. In a two-residue patch, each atom has a 1: or 2: prefix
      to identify the residue it refers to.

      \throws ValueException if the patch is not a two-residue patch.
   */
  void apply(CHARMMResidueTopology *res1, CHARMMResidueTopology *res2) const;
  IMP_OBJECT_METHODS(CHARMMPatch);
};

IMP_OBJECTS(CHARMMPatch, CHARMMPatches);

//! The topology of a single residue in a model.
/** Each CHARMMResidueTopology object can represent an 'unknown' residue
    (containing no atoms or bonds) or is a copy of an existing
    CHARMMIdealResidueTopology. Optionally, patches can be applied to
    residues (see CHARMMPatch::apply()) to add N- or C-termini, disulfide
    bridges, sidechain modifications, etc.
 */
class IMPATOMEXPORT CHARMMResidueTopology : public CHARMMIdealResidueTopology {
  bool patched_;

 public:

  //! Create an empty topology, containing no atoms or bonds.
  CHARMMResidueTopology(ResidueType type)
      : CHARMMIdealResidueTopology(type), patched_(false) {}

  //! Construct residue topology as a copy of an existing topology.
  CHARMMResidueTopology(CHARMMIdealResidueTopology *ideal)
      : CHARMMIdealResidueTopology(ideal->get_type()), patched_(false) {
    set_default_first_patch(ideal->get_default_first_patch());
    set_default_last_patch(ideal->get_default_last_patch());

    // Add atoms from existing topology
    for (unsigned int i = 0; i < ideal->get_number_of_atoms(); ++i) {
      add_atom(ideal->get_atom(i));
    }
    // Add angles/bonds/dihedrals/impropers
    for (unsigned int i = 0; i < ideal->get_number_of_bonds(); ++i) {
      add_bond(ideal->get_bond(i));
    }
    for (unsigned int i = 0; i < ideal->get_number_of_angles(); ++i) {
      add_angle(ideal->get_angle(i));
    }
    for (unsigned int i = 0; i < ideal->get_number_of_dihedrals(); ++i) {
      add_dihedral(ideal->get_dihedral(i));
    }
    for (unsigned int i = 0; i < ideal->get_number_of_impropers(); ++i) {
      add_improper(ideal->get_improper(i));
    }
    for (unsigned int i = 0; i < ideal->get_number_of_internal_coordinates();
         ++i) {
      add_internal_coordinate(ideal->get_internal_coordinate(i));
    }
  }

  bool get_patched() const { return patched_; }
  void set_patched(bool patched) { patched_ = patched; }

  IMP_OBJECT_METHODS(CHARMMResidueTopology);
};

IMP_OBJECTS(CHARMMResidueTopology, CHARMMResidueTopologies);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_CHARMM_TOPOLOGY_H */
