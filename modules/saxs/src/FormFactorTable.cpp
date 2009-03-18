/**
 *  \file FormFactorTable.h   \brief A class for computation of
 * atomic form factors for SAXS calculations
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/saxs/FormFactorTable.h>
#include <IMP/atom/AtomDecorator.h>
#include <IMP/constants.h>
#include <IMP/algebra/utility.h>

#include <fstream>
#include <algorithm>
#include <cmath>

IMPSAXS_BEGIN_NAMESPACE

FloatKey FormFactorTable::form_factor_key_ = FloatKey("form factor key");

String FormFactorTable::element_names_[] = {"H", "C", "N", "O", "S", "P", "AU"};

// electron density of solvent - default=0.334 e/A^3 (H2O)
Float FormFactorTable::rho_ = 0.334;

std::istream & operator>>(std::istream & s,
                          FormFactorTable::AtomFactorCoefficients &
                          atom_factor_coefficients)
{
  // read atom type
  s >> atom_factor_coefficients.atom_type_;
  // read ai values
  for (unsigned int i = 0; i < 5; i++) {
    s >> atom_factor_coefficients.a_[i];
  }
  s >> atom_factor_coefficients.c_;     // c value
  // read bi values
  for (unsigned int i = 0; i < 5; i++) {
    s >> atom_factor_coefficients.b_[i];
  }
  return s >> atom_factor_coefficients.excl_vol_;       // excluded volume
}


std::ostream & operator<<(std::ostream & s,
                          const FormFactorTable::AtomFactorCoefficients &
                          atom_factor_coefficients)
{
  s << atom_factor_coefficients.atom_type_ << ' ';
  for (unsigned int i = 0; i < 5; i++) {
    s << atom_factor_coefficients.a_[i] << ' ';
  }
  s << atom_factor_coefficients.c_ << ' ';
  for (unsigned int i = 0; i < 5; i++) {
    s << atom_factor_coefficients.b_[i] << ' ';
  }
  return s << atom_factor_coefficients.excl_vol_ << std::endl;
}


FormFactorTable::FormFactorTable(const String& table_name, Float min_q,
                                 Float max_q, Float delta_q):
  min_q_(min_q), max_q_(max_q), delta_q_(delta_q)
{
  // init all the tables
  form_factors_coefficients_ =
      std::vector < AtomFactorCoefficients > (ALL_ATOM_SIZE);
  unsigned int number_of_q_entries = algebra::round(
                                             (max_q_ - min_q_) / delta_q_ ) + 1;
  Floats form_factor_template(number_of_q_entries, 0.0);
  form_factors_ =
      std::vector < Floats > (HEAVY_ATOM_SIZE, form_factor_template);
  zero_form_factors_ = Floats(HEAVY_ATOM_SIZE, 0.0);

  // read ff table
  read_form_factor_table(table_name);

  // compute all the form factors
  compute_form_factors_all_atoms();
  compute_form_factors_heavy_atoms();
}


int FormFactorTable::read_form_factor_table(const String & table_name)
{
  std::ifstream s(table_name.c_str());
  if (!s) {
    std::
        cerr << "Can't find form factor table file " << table_name <<
        std::endl;
    exit(1);
  }
  // skip the comment lines
  char c;
  const int MAX_LENGTH = 1000;
  char line[MAX_LENGTH];
  while (s.get(c)) {
    if (c == '#') {         // if comment line, read the whole line and move on
      s.getline(line, MAX_LENGTH);
    } else {                    // return the first character
      s.putback(c);
      break;
    }
  }

  // read the data files
  AtomFactorCoefficients coeff;
  while (s >> coeff) {
    // find FormFactorAtomType
    for (unsigned int i = 0; i < ALL_ATOM_SIZE; i++) {
      if (element_names_[i] == coeff.atom_type_) {
        form_factors_coefficients_[i] = coeff;
        std::cerr << "read_form_factor_table: Atom type found: " <<
            coeff.atom_type_ << std::endl;
      }
    }
    //std::cerr << coeff << std::endl;
  }
  std::cerr << form_factors_coefficients_.size()
      << " form factors were read from file " << std::endl;
  return form_factors_coefficients_.size();
}


void FormFactorTable::show(std::ostream & out, std::string prefix) const
{
  for (unsigned int i = 0; i < zero_form_factors_.size(); i++) {
    out << prefix << " FFATOMTYPE " << i << " " << zero_form_factors_[i]
        << std::endl;
  }
}

/*
f(q) = f_atomic(q) - f_solvent(q)
f_atomic(q) = c + SUM [ a_i * EXP( - b_i * (q/4pi)^2 )]
                 i=1,5
f_solvent(q) = rho * v_i * EXP( (- v_i^(2/3) / (4pi)) * q^2 )
*/
void FormFactorTable::compute_form_factors_all_atoms()
{
  unsigned int number_of_q_entries =
    algebra::round((max_q_ - min_q_) / delta_q_ ) + 1;

  static Float two_third = 2.0 / 3;
  static Float one_over_four_pi = 1 / 4*PI;
  Floats qq(number_of_q_entries), ss(number_of_q_entries);

  // store qq and ss for the faster calculation
  for (unsigned int iq=0; iq<number_of_q_entries; iq++) {
    // the scattering vector q = (4pi) * sin(theta) / lambda
    Float q = min_q_ + (Float)iq * delta_q_;
    qq[iq] = square(q);       // qq = q^2

    // s = sin(theta) / lambda = q / (4pi), by Waasmaier and Kirfel (1995)
    Float s = q * one_over_four_pi;
    ss[iq] = square(s);       // ss = s^2 = (q/4pi)^2
  }

  for (unsigned int i = 0; i < ALL_ATOM_SIZE; i++) {
    // form factors for all the q range
    // volr_coeff = - v_i^(2/3) / 4PI
    Float volr_coeff = - std::pow(form_factors_coefficients_[i].excl_vol_,
                                  two_third)
                           * one_over_four_pi;

    for (unsigned int iq = 0; iq < number_of_q_entries; iq++) {
      // c
      form_factors_[i][iq] = form_factors_coefficients_[i].c_;

      // SUM [a_i * EXP( - b_i * (q/4pi)^2 )]
      for (unsigned int j = 0; j < 5; j++) {
        form_factors_[i][iq] += form_factors_coefficients_[i].a_[j]
                   * std::exp(-form_factors_coefficients_[i].b_[j] * ss[iq]);
      }
      // subtract solvation: rho * v_i * EXP( (- v_i^(2/3) / (4pi)) * q^2  )
      form_factors_[i][iq] -= rho_ * form_factors_coefficients_[i].excl_vol_
                          * std::exp(volr_coeff * qq[iq]);
    }
    // zero form factors
    zero_form_factors_[i] = form_factors_coefficients_[i].c_;
    for (unsigned int j = 0; j < 5; j++) {
      zero_form_factors_[i] += form_factors_coefficients_[i].a_[j];
    }
    // subtract solvation
    zero_form_factors_[i] -= rho_ * form_factors_coefficients_[i].excl_vol_;
  }
}


void FormFactorTable::compute_form_factors_heavy_atoms()
{
  unsigned int number_of_q_entries = algebra::round(
                                             (max_q_ - min_q_) / delta_q_ ) + 1;
  FormFactorAtomType element_type = UNK;
  unsigned int h_num = 0;       // bonded hydrogens number

  for (unsigned int i = ALL_ATOM_SIZE; i < HEAVY_ATOM_SIZE; i++) {
    switch (i) {
    case CH:
      element_type = C;
      h_num = 1;
      break;
    case CH2:
      element_type = C;
      h_num = 2;
      break;
    case CH3:
      element_type = C;
      h_num = 3;
      break;
    case NH:
      element_type = N;
      h_num = 1;
      break;
    case NH2:
      element_type = N;
      h_num = 2;
      break;
    case NH3:
      element_type = N;
      h_num = 3;
      break;
    case OH:
      element_type = O;
      h_num = 1;
      break;
    case SH:
      element_type = S;
      h_num = 1;
      break;
    default:
      break;
    }

    for (unsigned int iq = 0; iq < number_of_q_entries; iq++) {
      // ff(i) = ff(element) + h_num*ff(hydrogen)
      form_factors_[i][iq] =
          form_factors_[element_type][iq] + h_num * form_factors_[H][iq];
    }

    // zero form factors
    zero_form_factors_[i] =
        zero_form_factors_[element_type] + h_num * zero_form_factors_[H];
  }
}


FormFactorTable::FormFactorAtomType FormFactorTable::get_carbon_atom_type(
                       const atom::AtomType& atom_type,
                       const atom::ResidueType& residue_type) const {
  // CA
  if (atom_type == atom::AT_CA) {
    if (residue_type == atom::GLY)
      return CH2;                   // Glycine has 2 hydrogens
    return CH;
  }

  // CH3 at C-term
  //if(atom_type == atom::AT_CT) return CH3;

  // CH2
  if (atom_type == atom::AT_CH2) return CH2;

  // C
  if (atom_type == atom::AT_C) return C;

  // CB
  if (atom_type == atom::AT_CB) {
    if (residue_type == atom::ILE ||
        residue_type == atom::THR ||
        residue_type == atom::VAL) return CH;
    if (residue_type == atom::ALA) return CH3;
    return CH2;
  }

  // CG1
  if (atom_type == atom::AT_CG) {
    if (residue_type == atom::ASN ||
        residue_type == atom::ASP ||
        residue_type == atom::HIS ||
        residue_type == atom::PHE ||
        residue_type == atom::TRP ||
        residue_type == atom::TYR) return C;
    if (residue_type == atom::LEU) return CH;
    return CH2;
  }

  // CG1
  if (atom_type == atom::AT_CG1) {
    if (residue_type == atom::ILE) return CH2;
    if (residue_type == atom::VAL) return CH3;
  }

  // CG2 - only VAL, ILE, and THR
  if (atom_type == atom::AT_CG2) return CH3;

  // CD
  if (atom_type == atom::AT_CD) {
    if (residue_type == atom::GLU ||
        residue_type == atom::GLN) return C;
    return CH2;
  }

  // CD1
  if (atom_type == atom::AT_CD1) {
    if (residue_type == atom::LEU ||
        residue_type == atom::ILE) return CH3;
    if (residue_type == atom::PHE ||
        residue_type == atom::TRP ||
        residue_type == atom::TYR) return CH;
    return C;
  }

  // CD2
  if (atom_type == atom::AT_CD2) {
    if (residue_type == atom::LEU) return CH3;
    if (residue_type == atom::PHE ||
        residue_type == atom::HIS ||
        residue_type == atom::TYR) return CH;
    return C;
  }

  // CE
  if (atom_type == atom::AT_CE) {
    if (residue_type == atom::LYS) return CH2;
    if (residue_type == atom::MET) return CH3;
    return C;
  }

  // CE1
  if (atom_type == atom::AT_CE1) {
    if (residue_type == atom::PHE ||
        residue_type == atom::HIS ||
        residue_type == atom::TYR) return CH;
    return C;
  }

  // CE2
  if (atom_type == atom::AT_CE2) {
    if (residue_type == atom::PHE ||
        residue_type == atom::TYR) return CH;
    return C;
  }
  // CZ
  if (atom_type == atom::AT_CZ) {
    if (residue_type == atom::PHE) return CH;
    return C;
  }
//   // CZ1
//   if(atom_type == atom::AT_CZ1) return C;

  // CZ2, CZ3, CE3
  if (atom_type == atom::AT_CZ2 ||
      atom_type == atom::AT_CZ3 ||
      atom_type == atom::AT_CE3) {
    if (residue_type == atom::TRP) return CH;
    return C;
  }

  std::cerr << "Error in get_carbon_atom_type: atom not found "
      << atom_type << " " << residue_type << std::endl;
  return C;
}


FormFactorTable::FormFactorAtomType FormFactorTable::get_nitrogen_atom_type(
                     const atom::AtomType& atom_type,
                     const atom::ResidueType& residue_type) const {
  // N
  if (atom_type == atom::AT_N) {
    if (residue_type == atom::PRO) return N;
    return NH;
  }
  //// ND
  //if(atom_type == atom::AT_ND) return N;
  // ND1
  if (atom_type == atom::AT_ND1) {
    if (residue_type == atom::HIS) return NH;
    return N;
  }
  // ND2
  if (atom_type == atom::AT_ND2) {
    if (residue_type == atom::ASN) return NH2;
    return N;
  }
  // NH1, NH2
  if (atom_type == atom::AT_NH1 ||
      atom_type == atom::AT_NH2) {
    if (residue_type == atom::ARG) return NH2;
    return N;
  }
  // NE
  if (atom_type == atom::AT_NE) {
    if (residue_type == atom::ARG) return NH;
    return N;
  }
  // NE1
  if (atom_type == atom::AT_NE1) {
    if (residue_type == atom::TRP) return NH;
    return N;
  }
  // NE2
  if (atom_type == atom::AT_NE2) {
    if (residue_type == atom::GLN) return NH2;
    return N;
  }
  // NZ
  if (atom_type == atom::AT_NZ) {
    if (residue_type == atom::LYS) return NH3;
    return N;
  }
  std::cerr << "Error in get_nitrogen_atom_type: atom not found "
      << atom_type << " " << residue_type << std::endl;
  return N;
}


FormFactorTable::FormFactorAtomType FormFactorTable::get_oxygen_atom_type(
                     const atom::AtomType& atom_type,
                     const atom::ResidueType& residue_type) const {

  // O OE1 OE2 OD1 OD2 O1A O2A OXT OT1 OT2
  if (atom_type == atom::AT_O ||
      atom_type == atom::AT_OE1 ||
      atom_type == atom::AT_OE2 ||
      atom_type == atom::AT_OD1 ||
      atom_type == atom::AT_OD2 ||
      //atom_type == atom::AT_O1A ||
      //atom_type == atom::AT_O2A ||
      //atom_type == atom::AT_OT1 ||
      //atom_type == atom::AT_OT2 ||
      atom_type == atom::AT_OXT) return O;
  // OG
  if (atom_type == atom::AT_OG) {
    if (residue_type == atom::SER) return OH;
    return O;
  }
  // OG1
  if (atom_type == atom::AT_OG1) {
    if (residue_type == atom::THR) return OH;
    return O;
  }
  // OH
  if (atom_type == atom::AT_OH) {
    if (residue_type == atom::TYR) return OH;
    return O;
  }
  std::cerr << "Error in get_oxygen_atom_type: atom not found "
      << atom_type << " " << residue_type << std::endl;
  return O;
}


FormFactorTable::FormFactorAtomType FormFactorTable::get_sulfur_atom_type(
                     const atom::AtomType& atom_type,
                     const atom::ResidueType& residue_type) const {
  // SD
  if (atom_type == atom::AT_SD) return S;
  // SG
  if (atom_type == atom::AT_SG) {
    if (residue_type == atom::CYS) return SH;
    return S;
  }
  std::cerr << "Error in get_sulfur_atom_type: atom not found "
      << atom_type << " " << residue_type << std::endl;
  return S;
}

FormFactorTable::FormFactorAtomType FormFactorTable::get_form_factor_atom_type(
                                   Particle * p, FormFactorType ff_type) const {
  atom::ResidueType residue_type =
    atom::get_residue_type(atom::AtomDecorator::cast(p));
  atom::AtomType atom_type =
      atom::AtomDecorator::cast(p).get_type();
  String atom_name = atom_type.get_string();

  FormFactorAtomType ret_type = UNK;
  int name_start = 0;
  if (isdigit(atom_name[0]))
    name_start = 1;
  for (unsigned int i = 0; i < ALL_ATOM_SIZE; i++) {
    int comp_length = std::min(atom_name.length(), element_names_[i].length());
    if (atom_name.substr(name_start, comp_length) == element_names_[i]) {
      ret_type = (FormFactorAtomType) i;
    }
  } if (ff_type == HEAVY_ATOMS) {
    switch (ret_type) {
    case C:
      ret_type = get_carbon_atom_type(atom_type, residue_type);
      break;
    case N:
      ret_type = get_nitrogen_atom_type(atom_type, residue_type);
      break;
    case O:
      ret_type = get_oxygen_atom_type(atom_type, residue_type);
      break;
    case S:
      ret_type = get_sulfur_atom_type(atom_type, residue_type);
      break;
    default:
      break;
    }
  }
  //std::cerr << "FormFactorAtomType = " << ret_type << " "
  //        << atom_type << " " << residue_type << std::endl;
  return ret_type;
}


Float FormFactorTable::get_form_factor(Particle * p,
                                       FormFactorType ff_type) const {
  // initialization by request
  if (p->has_attribute(form_factor_key_))
    return p->get_value(form_factor_key_);

  FormFactorAtomType ff_atom_type = get_form_factor_atom_type(p, ff_type);
  Float form_factor = zero_form_factors_[(int)ff_atom_type];
  //std::cerr << "form_factor " << form_factor << std::endl;
  p->add_attribute(form_factor_key_, form_factor);
  return form_factor;
}


const Floats& FormFactorTable::get_form_factors(Particle * p,
                                                FormFactorType ff_type) const {
  FormFactorAtomType ff_atom_type = get_form_factor_atom_type(p, ff_type);
  return form_factors_[(int)ff_atom_type];
}

IMPSAXS_END_NAMESPACE
