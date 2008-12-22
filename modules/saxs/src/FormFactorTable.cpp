/**
 *  \file FormFactorTable.h   \brief A class for computation of
 * atomic form factors for SAXS calculations
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#include "FormFactorTable.h"

istream& operator>>(istream& s,
    FormFactorTable::AtomFactorCoefficients& atom_factor_coefficients) {
  // read atom type
  s >> atom_factor_coefficients.atom_type_;

  float val;
  // read ai values
  for(unsigned int i=0; i<5; i++) {
    s >> val;
    atom_factor_coefficients.a_.push_back(val);
  }
  s >> atom_factor_coefficients.c_; // c value
  // read bi values
  for(unsigned int i=0; i<5; i++) {
    s >> val;
    atom_factor_coefficients.b_.push_back(val);
  }
  return s >> atom_factor_coefficients.excl_vol_; // excluded volume
}

ostream& operator<<(ostream& s,
   const FormFactorTable::AtomFactorCoefficients& atom_factor_coefficients) {
  s << atom_factor_coefficients.atom_type_ << ' ';
  for(unsigned int i=0; i<5; i++) {
    s << atom_factor_coefficients.a_[i] << ' ';
  }
  s << atom_factor_coefficients.c_ << ' ';
  for(unsigned int i=0; i<5; i++) {
    s << atom_factor_coefficients.b_[i] << ' ';
  }
  return s << atom_factor_coefficients.excl_vol_ << endl;
}

int FormFactorTable::readFormFactorTable(istream& s) {

  // skip the comment lines
  char c;
  int MAX_LENGTH = 1000;
  char line[MAX_LENGTH];
  while(s.get(c)) {
    if (c == '#') { // if comment line, read the whole line and move on
      s.getline(line, MAX_LENGTH);
    } else {  // return the first character
      s.putback(c);
      break;
    }
  }

  // read the data files
  AtomFactorCoefficients coeff;
  while(s >> coeff) {
    form_factors_coefficients_.push_back(coeff);
    cerr << coeff << endl;
  }
  cerr << form_factors_coefficients_.size()
       << " form factors were read from file " << endl;
  return form_factors_coefficients_.size();
}
