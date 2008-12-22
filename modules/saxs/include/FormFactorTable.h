/**
 *  \file FormFactorTable.h   \brief A class for computation of
 * atomic form factors for SAXS calculations
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPSAXS_FORM_FACTOR_TABLE_H
#define IMPSAXS_FORM_FACTOR_TABLE_H

#include <IMP/Particle.h>

#include <iostream>
#include <string>
#include <vector>

using std::istream;
using std::ostream;
using std::string;
using std::vector;
using std::endl;
using std::cerr;


// atom types for heavy atoms according to the number of hydrogens
// connected to them
enum FFHeavyAtomType
  {C, CH, CH2, CH3, N, NH, NH2, NH3, O, OH, S, SH, P, AU, UNK};

enum FFType { ALL_ATOMS, HEAVY_ATOMS, CA_ATOMS };

/*
   class that deals with form factor computation
*/
class FormFactorTable {
public:
  FormFactorTable() {};

  int readFormFactorTable(istream& s);

  // get f(0), ie s=0
  float getFormFactor(IMP::Particle* p, FFType ff_type = HEAVY_ATOMS);

  //
  const vector<float>& getFormFactors(IMP::Particle* p,
                                      FFType ff_type = HEAVY_ATOMS) const;

  class AtomFactorCoefficients {
  public:
    // read entry
    friend istream& operator>>(istream& s,
                             AtomFactorCoefficients& atom_factor_coefficients);

    // write entry
    friend ostream& operator<<(ostream& s,
                       const AtomFactorCoefficients& atom_factor_coefficients);

  public:
    string atom_type_;
    vector<float> a_;
    vector<float> b_;
    float c_;
    float excl_vol_;
  };

private:
  void computeFormFactors();
  void computeFormFactorsAllAtoms();
  void computeFormFactorsHeavyAtoms();
  void computeFormFactorsCAAtoms();

  // 3 types of form factors
  const vector<float>& getFormFactorsAllAtom(IMP::Particle* p) const;
  const vector<float>& getFormFactorsHeavyAtom(IMP::Particle* p) const;
  const vector<float>& getFormFactorsCAAtom(IMP::Particle* p) const;

  // only s=0 form factor for faster computation
  float getFormFactorAllAtom(IMP::Particle* p) const;
  float getFormFactorHeavyAtom(IMP::Particle* p) const;
  float getFormFactorCAAtom(IMP::Particle* p) const;

protected:
  // read from lib file
  vector<AtomFactorCoefficients> form_factors_coefficients_;

  // tables of form factors for 14 atom types
  vector<vector<float> > form_factors_all_atoms_;
  vector<vector<float> > form_factors_heavy_atoms_;
  vector<vector<float> > form_factors_CA_atoms_;

  // tables for s=0
  vector<float> zero_form_factors_all_atoms_;
  vector<float> zero_form_factors_heavy_atoms_;
  vector<float> zero_form_factors_CA_atoms_;

  float s_min_, s_max_; // max s for form factor computation
  float delta_s_; // resolution

  //static IntKey ff_atom_type_;

};

#endif /* IMPSAXS_FORM_FACTOR_TABLE_H */
