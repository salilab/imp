/**
 * \file IMP/atom/ForceFieldParameters.h \brief force field base class
 *
 * Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_FORCE_FIELD_PARAMETERS_H
#define IMPATOM_FORCE_FIELD_PARAMETERS_H

#include "Residue.h"
#include "Atom.h"
#include "Hierarchy.h"
#include <IMP/base_types.h>
#include <IMP/atom/LennardJonesType.h>

IMPATOM_BEGIN_NAMESPACE

//! Storage and access to force field
class IMPATOMEXPORT ForceFieldParameters : public IMP::Object {
 public:
  ForceFieldParameters() : Object("ForceFieldParameters%1%") {}
  //! get radius
  Float get_radius(Atom atom) const;

  //! get epsilon for non bonded vdW
  Float get_epsilon(Atom atom) const;

  //! Add or replace radii in the structure defined in the hierarchy.
  /** Each radius is scaled by the given scale.
   */
  void add_radii(Hierarchy mhd, double scale = 1.0,
                 FloatKey radius_key = FloatKey("radius")) const;

  //! Add Lennard-Jones types to the structure
  /** This adds LennardJonesTyped decorators to all applicable particles. */
  void add_lennard_jones_types(Hierarchy mhd) const;

  //! add bonds to the structure defined in the hierarchy
  void add_bonds(Hierarchy mhd) const;

  IMP_REF_COUNTED_DESTRUCTOR(ForceFieldParameters);

 protected:
  /** Store endpoints for a bond. */
  class Bond {
   public:
    Bond(AtomType type1, AtomType type2,
         IMP::atom::Bond::Type bond_type = IMP::atom::Bond::SINGLE)
        : type1_(type1), type2_(type2), bond_type_(bond_type) {}
    AtomType type1_, type2_;
    IMP::atom::Bond::Type bond_type_;
  };

  Float get_radius(const String& force_field_atom_type) const;
  Float get_epsilon(const String& force_field_atom_type) const;
  LennardJonesType *get_lj_type(const String& force_field_atom_type) const;
  virtual String get_force_field_atom_type(Atom atom) const;
  void add_bonds(Residue rd) const;
  void add_bonds(Residue rd1, Residue rd2) const;

  // map between imp_atom_type and force_field parameters (atom_type, charge)
  typedef std::pair<std::string, double> ChargePair;
  typedef std::map<AtomType, ChargePair> AtomTypeMap;

  // hash that maps between residue and atom name to force_field atom type
  // key1=imp_residue_type, key2= imp_atom_type, value=pair(atom_type, charge)
  std::map<ResidueType, AtomTypeMap> atom_res_type_2_force_field_atom_type_;

  // a list of residue bonds
  std::map<ResidueType, Vector<Bond>> residue_bonds_;

  // map that holds force_field parameters according to force_field atom types
  // key=force_field_atom_type, value=(epsilon,radius)
  std::map<String, FloatPair> force_field_2_vdW_;

  // mapping from parameters to instantianted Lennard Jones types
  mutable std::map<String, Pointer<LennardJonesType>> force_field_2_lj_type_;

 private:
  WarningContext warn_context_;
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_FORCE_FIELD_PARAMETERS_H */
