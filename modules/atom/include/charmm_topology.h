/**
 * \file charmm_topology.h \brief Classes for handling CHARMM-style topology.
 *
 * Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPATOM_CHARMM_TOPOLOGY_H
#define IMPATOM_CHARMM_TOPOLOGY_H

#include "IMP/Object.h"
#include "IMP/container_macros.h"
#include "Hierarchy.h"
#include "Atom.h"
#include "config.h"

#include <string>
#include <vector>

IMPATOM_BEGIN_NAMESPACE

//! A single atom in a CHARMM topology.
/** Each atom has a name (unique to the residue), a CHARMM type (used to
    look up parameters such as radii and bond lengths in the parameter file)
    and an electrostatic charge.
 */
class CHARMMAtomTopology {
  std::string name_;
  std::string charmm_type_;
  double charge_;
public:
  CHARMMAtomTopology(std::string name) : name_(name) {};

  std::string get_name() const { return name_; }
  std::string get_charmm_type() const { return charmm_type_; }
  double get_charge() const { return charge_; }
  void set_charmm_type(std::string charmm_type) { charmm_type_ = charmm_type; }
  void set_charge(double charge) { charge_ = charge; }
};

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
//CHARMMResidueTopology *residue_;
public:
  CHARMMBondEndpoint(std::string atom_name) : atom_name_(atom_name) {}

  std::string get_atom_name() const { return atom_name_; }
};

//! A bond, angle, dihedral or improper between some number of endpoints.
template <unsigned int D>
class CHARMMBond
{
  std::vector<CHARMMBondEndpoint> endpoints_;
public:
  CHARMMBond(std::vector<std::string> atoms) {
    IMP_INTERNAL_CHECK(atoms.size() == D, "wrong number of bond endpoints");
    for (std::vector<std::string>::const_iterator it = atoms.begin();
         it != atoms.end(); ++it) {
      endpoints_.push_back(CHARMMBondEndpoint(*it));
    }
  }
  bool contains_atom(std::string name) const {
    for (std::vector<CHARMMBondEndpoint>::const_iterator
         it = endpoints_.begin(); it != endpoints_.end(); ++it) {
      if (it->get_atom_name() == name) {
        return true;
      }
    }
    return false;
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

  void add_atom(const CHARMMAtomTopology &atom);
  CHARMMAtomTopology &get_atom(std::string name);
  CHARMMAtomTopology &get_atom(AtomType type) {
    return get_atom(type.get_string());
  }

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

//! The ideal topology of a single residue as read from a CHARMM topology file
class IMPATOMEXPORT CHARMMIdealResidueTopology
    : public CHARMMResidueTopologyBase {
  std::string default_first_patch_, default_last_patch_;
public:
  CHARMMIdealResidueTopology(std::string type)
      : CHARMMResidueTopologyBase(type) {}

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

//! A CHARMM patch residue
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

  void apply(CHARMMResidueTopology &res) const;
  // Todo: handle two-residue patches (DISU, LINK)
  // void apply(CHARMMResidueTopology &res1, CHARMMResidueTopology &res2);
};

//! The topology of a single residue in a model
class IMPATOMEXPORT CHARMMResidueTopology
     : public CHARMMIdealResidueTopology, public Object {
  bool patched_;
public:

  CHARMMResidueTopology(const CHARMMIdealResidueTopology &ideal)
    : CHARMMIdealResidueTopology(ideal), patched_(false) {}

  bool get_patched() const { return patched_; }
  void set_patched(bool patched) { patched_ = patched; }

  IMP_OBJECT(CHARMMResidueTopology);
};

IMP_OBJECTS(CHARMMResidueTopology);

class CharmmParameters;

//! The topology of a single CHARMM segment (chain) in a model
class IMPATOMEXPORT CHARMMSegmentTopology : public Object {
  IMP_LIST(public, CHARMMResidueTopology, residue, CHARMMResidueTopology*,
           CHARMMResidueTopologys);

  IMP_OBJECT(CHARMMSegmentTopology);
public:
  void apply_default_patches(const CharmmParameters *ff);
};

IMP_OBJECTS(CHARMMSegmentTopology);

//! The topology of a complete CHARMM model
class IMPATOMEXPORT CHARMMTopology : public Object {
  IMP_LIST(public, CHARMMSegmentTopology, segment, CHARMMSegmentTopology*,
           CHARMMSegmentTopologys);

  IMP_OBJECT(CHARMMTopology);
private:
  WarningContext warn_context_;

  void map_residue_topology_to_hierarchy(Hierarchy hierarchy,
            std::map<CHARMMResidueTopology *, Hierarchy> &resmap) const;
public:
  void apply_default_patches(const CharmmParameters *ff) {
    for (unsigned int i = 0; i < get_number_of_segments(); ++i) {
      get_segment(i)->apply_default_patches(ff);
    }
  }

  void add_atom_types(Hierarchy hierarchy) const;

  void add_charges(Hierarchy hierarchy) const;
};

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_CHARMM_TOPOLOGY_H */
