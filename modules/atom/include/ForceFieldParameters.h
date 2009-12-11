/**
 * \file ForceFieldParameters.h \brief force field base class
 *
 * Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPATOM_FORCE_FIELD_PARAMETERS_H
#define IMPATOM_FORCE_FIELD_PARAMETERS_H

#include "Residue.h"
#include "Atom.h"
#include "Hierarchy.h"
#include <IMP/base_types.h>

IMPATOM_BEGIN_NAMESPACE

//! Storage and access to force field
class IMPATOMEXPORT ForceFieldParameters: public Object {
public:

  //! get radius
  Float get_radius(AtomType atom_type,
                   ResidueType residue_type) const;

  //! get epsilon for non bonded vdW
  Float get_epsilon(AtomType atom_type,
                    ResidueType residue_type) const;

  //! add radii to the structure defined in the hierarchy
  void add_radii(Hierarchy mhd, FloatKey radius_key= FloatKey("radius")) const;

  //! add bonds to the structure defined in the hierarchy
  void add_bonds(Hierarchy mhd) const;

  IMP_REF_COUNTED_DESTRUCTOR(ForceFieldParameters);
protected:
  class Bond {
  public:
    Bond(AtomType type1, AtomType type2,
         IMP::atom::Bond::Type bond_type = IMP::atom::Bond::COVALENT) :
      type1_(type1), type2_(type2), bond_type_(bond_type) {}
    AtomType type1_,type2_;
    IMP::atom::Bond::Type bond_type_;
  };

  Float get_radius(const String& force_field_atom_type) const;
  Float get_epsilon(const String& force_field_atom_type) const;
  String get_force_field_atom_type(AtomType atom_type,
                                   ResidueType residue_type) const;
  void add_bonds(Residue rd) const;
  void add_bonds(Residue rd1, Residue rd2) const;


  // map between imp_atom_type and force_field parameters (atom_type, charge)
  typedef std::map<AtomType, std::pair<std::string, float> > AtomTypeMap;

  // hash that maps between residue and atom name to force_field atom type
  // key1=imp_residue_type, key2= imp_atom_type, value=pair(atom_type, charge)
  std::map<ResidueType, AtomTypeMap> atom_res_type_2_force_field_atom_type_;

  // a list of residue bonds
  std::map<ResidueType, std::vector<Bond> > residue_bonds_;

  // map that holds force_field parameters according to force_field atom types
  // key=force_field_atom_type, value=(epsilon,radius)
  std::map<String, std::pair<float, float> > force_field_2_vdW_;
};


IMPATOMEXPORT ForceFieldParameters* default_force_field_parameters();

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_FORCE_FIELD_PARAMETERS_H */
