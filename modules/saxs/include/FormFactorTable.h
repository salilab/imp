/**
 *  \file FormFactorTable.h   \brief A class for computation of
 * atomic form factors for SAXS calculations
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPSAXS_FORM_FACTOR_TABLE_H
#define IMPSAXS_FORM_FACTOR_TABLE_H

#include "config.h"

#include <IMP/Particle.h>
#include <IMP/base_types.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/element.h>
#include <IMP/algebra/utility.h>

#include <iostream>
#include <vector>

IMPSAXS_BEGIN_NAMESPACE

/**
class that deals with form factor computation
*/
class IMPSAXSEXPORT FormFactorTable: public RefCounted {
public:
  //! default constructor
  FormFactorTable();

  //! constructor with form factor table file
  FormFactorTable(const String& table_name, Float min_q, Float max_q,
                  Float delta_q);

  //! type of the form factors for profile calculations
  enum FormFactorType { ALL_ATOMS, HEAVY_ATOMS };

  //! get f(0), ie q=0 for real space profile calculation
  Float get_form_factor(Particle* p, FormFactorType ff_type=HEAVY_ATOMS) const;

  //! for reciprocal space profile calculation
  const Floats& get_form_factors(Particle* p,
                                 FormFactorType ff_type = HEAVY_ATOMS) const;

  //! print tables
  void show(std::ostream &out=std::cout, std::string prefix="") const;

  // electron density of solvent - default=0.334 e/A^3 (H2O)
  static Float rho_;
  IMP_REF_COUNTED_DESTRUCTOR(FormFactorTable);
private:
  // atom types for heavy atoms according to the number of hydrogens
  // connected to them
  // ALL_ATOM_SIZE is number of types needed for all atom representation
  // this indexing is used in form_factors arrays
  enum FormFactorAtomType {
    H, C, N, O, S, P, He, Ne, Na, Mg, Ca, Fe, Zn, Se, Au, ALL_ATOM_SIZE = 15,
    CH=15, CH2=16, CH3=17, NH=18, NH2=19, NH3=20, OH=21, SH=22,
    HEAVY_ATOM_SIZE=23, UNK=24};

  // map between atom element and FormFactorAtomType
  static std::map<atom::Element, FormFactorAtomType> element_ff_type_map_;

  // form factors for q=0, the order as in the FormFactorAtomType enum
  static Float zero_form_factors_[];

  // a key for storing zero form factor in Particle as attribute
  static IntKey form_factor_key_;

  // class for storing form factors solvation table
  class AtomFactorCoefficients {
  public:
    String atom_type_;
    Float a_[5];
    Float b_[5];
    Float c_;
    Float excl_vol_;
  };

  // read entry
  friend std::istream& operator>>(std::istream& s,
                             AtomFactorCoefficients& atom_factor_coefficients);

  // write entry
  friend std::ostream& operator<<(std::ostream& s,
                       const AtomFactorCoefficients& atom_factor_coefficients);

private:
  int read_form_factor_table(const String& table_name);

  void init_element_form_factor_map();

  void compute_form_factors_all_atoms();

  void compute_form_factors_heavy_atoms();

  FormFactorAtomType get_form_factor_atom_type(atom::Element e) const;

  FormFactorAtomType get_form_factor_atom_type(Particle* p,
                                               FormFactorType ff_type) const;

  FormFactorAtomType get_carbon_atom_type(const atom::AtomType& atom_type,
                                const atom::ResidueType& residue_type) const;

  FormFactorAtomType get_nitrogen_atom_type(const atom::AtomType& atom_type,
                                const atom::ResidueType& residue_type) const;

  FormFactorAtomType get_oxygen_atom_type(const atom::AtomType& atom_type,
                                const atom::ResidueType& residue_type) const;

  FormFactorAtomType get_sulfur_atom_type(const atom::AtomType& atom_type,
                                const atom::ResidueType& residue_type) const;

private:
  // read from lib file
  std::vector<AtomFactorCoefficients> form_factors_coefficients_;

  // table of form factors for 14 atom types
  std::vector<Floats> form_factors_;

  // min/max q and sampling resolution for form factor computation
  Float min_q_, max_q_, delta_q_;
};

IMPSAXSEXPORT FormFactorTable* default_form_factor_table();

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_FORM_FACTOR_TABLE_H */
