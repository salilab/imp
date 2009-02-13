/**
 *  \file FormFactorTable.h   \brief A class for computation of
 * atomic form factors for SAXS calculations
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPSAXS_FORM_FACTOR_TABLE_H
#define IMPSAXS_FORM_FACTOR_TABLE_H

#include "config.h"

#include <IMP/Particle.h>
#include <IMP/base_types.h>
#include <IMP/core/ResidueDecorator.h>
#include <IMP/core/AtomDecorator.h>
#include <IMP/algebra/utility.h>

#include <iostream>
#include <vector>

IMPSAXS_BEGIN_NAMESPACE

/*
   class that deals with form factor computation
*/
class IMPSAXSEXPORT FormFactorTable {
public:

  //! constructor
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

private:
  // atom types for heavy atoms according to the number of hydrogens
  // connected to them
  // ALL_ATOM_SIZE is number of types needed for all atom representation
  // this indexing is used in form_factors arrays
  enum FormFactorAtomType
    {H=0, C=1, N=2, O=3, S=4, P=5, AU=6, ALL_ATOM_SIZE = 7,
     CH=7, CH2=8, CH3=9, NH=10, NH2=11, NH3=12, OH=13, SH=14,
     HEAVY_ATOM_SIZE=15, UNK=16};

  // the names correspond to the first FormFactorAtomTypes
  // (the order should be the same)
  static String element_names_[];// {"H", "C", "N", "O", "S", "P", "AU"};

  // a key for storing zero form factor in Particle as attribute
  static FloatKey form_factor_key_;

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

  void compute_form_factors_all_atoms();

  void compute_form_factors_heavy_atoms();

  FormFactorAtomType get_form_factor_atom_type(Particle* p,
                                               FormFactorType ff_type) const;

  FormFactorAtomType get_carbon_atom_type(const core::AtomType& atom_type,
                                const core::ResidueType& residue_type) const;

  FormFactorAtomType get_nitrogen_atom_type(const core::AtomType& atom_type,
                                const core::ResidueType& residue_type) const;

  FormFactorAtomType get_oxygen_atom_type(const core::AtomType& atom_type,
                                const core::ResidueType& residue_type) const;

  FormFactorAtomType get_sulfur_atom_type(const core::AtomType& atom_type,
                                const core::ResidueType& residue_type) const;

private:
  // read from lib file
  std::vector<AtomFactorCoefficients> form_factors_coefficients_;

  // table of form factors for 14 atom types
  std::vector<Floats> form_factors_;

  // form factors for q=0
  Floats zero_form_factors_;

  // min/max q and sampling resolution for form factor computation
  Float min_q_, max_q_, delta_q_;
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_FORM_FACTOR_TABLE_H */
