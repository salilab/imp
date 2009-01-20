/**
 *  \file FormFactorTable.h   \brief A class for computation of
 * atomic form factors for SAXS calculations
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/saxs/FormFactorTable.h>

#include <IMP/core/AtomDecorator.h>

#include <fstream>
#include <algorithm>

IMPSAXS_BEGIN_NAMESPACE

FloatKey FormFactorTable::form_factor_key_ = FloatKey("form factor key");

String FormFactorTable::element_names[] = {"H", "C", "N", "O", "S", "P", "AU"};

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

FormFactorTable::FormFactorTable(const String& table_name, Float min_s,
                                 Float max_s, Float delta_s):
  min_s_(min_s), max_s_(max_s), delta_s_(delta_s)
{

  // init all the tables
  form_factors_coefficients_ =
      std::vector < AtomFactorCoefficients > (ALL_ATOM_SIZE);
  unsigned int number_of_entries = (max_s_ - min_s_) / delta_s_ + 1;
  Floats form_factor_template(number_of_entries, 0.0);
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
  int MAX_LENGTH = 1000;
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
      if (element_names[i] == coeff.atom_type_) {
        form_factors_coefficients_[i] = coeff;
        std::cerr << "read_form_factor_table: Atom type found: " <<
            coeff.atom_type_ << std::endl;
      }
    }
    //    cerr << coeff << endl;
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

  f_atomic(q) = c + [SUM a_i*EXP(-b_i*(q^2))]
                    i=1,5

  f_solvent(q) = v_i * EXP(-4PI * v_i^(2/3) * q^2)

  q is divided by 4PI
*/
void FormFactorTable::compute_form_factors_all_atoms()
{
  unsigned int number_of_entries = (max_s_ - min_s_) / delta_s_ + 1;

  // electron density of solvent - default=0.334 e/A^3 (H2O)
  float rho = 0.334;

  for (unsigned int i = 0; i < ALL_ATOM_SIZE; i++) {
    // form factors for all the q range
    Float volr = pow(form_factors_coefficients_[i].excl_vol_, (float)2.0 / 3) /
      4 * M_PI;    // v_i^(2/3) / 4PI
    for (unsigned int k = 0; k < number_of_entries; k++) {
      Float q = min_s_ + k * delta_s_;
      Float s = square(q / 4 * M_PI); // (q/4PI)^2

      // c
      form_factors_[i][k] = form_factors_coefficients_[i].c_;

      // [SUM a_i*EXP(-b_i*(q^2))]
      //  i=1,5
      for (unsigned int j = 0; j < 5; j++) {
        form_factors_[i][k] += form_factors_coefficients_[i].a_[j] *    // a_i
            exp(-form_factors_coefficients_[i].b_[j] * s);   // EXP(-b_i*(q^2))
      }

      // subtract solvation: pho*v_i*EXP(-4PI * v_i^(2/3) * q^2)
      form_factors_[i][k] -=
          rho * form_factors_coefficients_[i].excl_vol_ * exp(-volr * q * q);
    }

    // zero form factors
    zero_form_factors_[i] = form_factors_coefficients_[i].c_;
    for (unsigned int j = 0; j < 5; j++) {
      zero_form_factors_[i] += form_factors_coefficients_[i].a_[j];
    }
    // subtract solvation
    zero_form_factors_[i] -= rho * form_factors_coefficients_[i].excl_vol_;
  }
}

void FormFactorTable::compute_form_factors_heavy_atoms()
{
  unsigned int number_of_entries = (max_s_ - min_s_) / delta_s_ + 1;

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

    for (unsigned int k = 0; k < number_of_entries; k++) {
      // ff(i) = ff(element) + h_num*ff(hydrogen)
      form_factors_[i][k] =
          form_factors_[element_type][k] + h_num * form_factors_[H][k];
    }

    // zero form factors
    zero_form_factors_[i] =
        zero_form_factors_[element_type] + h_num * zero_form_factors_[H];
  }
}

FormFactorTable::FormFactorAtomType FormFactorTable::get_carbon_atom_type(
                       const core::AtomType& atom_type,
                       const core::ResidueType& residue_type) const {
  // CA
  if (atom_type == core::AtomDecorator::AT_CA) {
    if (residue_type == core::ResidueDecorator::GLY)
      return CH2;                   // Glycine has 2 hydrogens
    return CH;
  }

  // CH3 at C-term
  //if(atom_type == core::AtomDecorator::AT_CT) return CH3;

  // CH2
  if (atom_type == core::AtomDecorator::AT_CH2) return CH2;

  // C
  if (atom_type == core::AtomDecorator::AT_C) return C;

  // CB
  if (atom_type == core::AtomDecorator::AT_CB) {
    if (residue_type == core::ResidueDecorator::ILE ||
        residue_type == core::ResidueDecorator::THR ||
        residue_type == core::ResidueDecorator::VAL) return CH;
    if (residue_type == core::ResidueDecorator::ALA) return CH3;
    return CH2;
  }

  // CG1
  if (atom_type == core::AtomDecorator::AT_CG) {
    if (residue_type == core::ResidueDecorator::ASN ||
        residue_type == core::ResidueDecorator::ASP ||
        residue_type == core::ResidueDecorator::HIS ||
        residue_type == core::ResidueDecorator::PHE ||
        residue_type == core::ResidueDecorator::TRP ||
        residue_type == core::ResidueDecorator::TYR) return C;
    if (residue_type == core::ResidueDecorator::LEU) return CH;
    return CH2;
  }

  // CG1
  if (atom_type == core::AtomDecorator::AT_CG1) {
    if (residue_type == core::ResidueDecorator::ILE) return CH2;
    if (residue_type == core::ResidueDecorator::VAL) return CH3;
  }

  // CG2 - only VAL, ILE, and THR
  if (atom_type == core::AtomDecorator::AT_CG2) return CH3;

  // CD
  if (atom_type == core::AtomDecorator::AT_CD) {
    if (residue_type == core::ResidueDecorator::GLU ||
        residue_type == core::ResidueDecorator::GLN) return C;
    return CH2;
  }

  // CD1
  if (atom_type == core::AtomDecorator::AT_CD1) {
    if (residue_type == core::ResidueDecorator::LEU ||
        residue_type == core::ResidueDecorator::ILE) return CH3;
    if (residue_type == core::ResidueDecorator::PHE ||
        residue_type == core::ResidueDecorator::TRP ||
        residue_type == core::ResidueDecorator::TYR) return CH;
    return C;
  }

  // CD2
  if (atom_type == core::AtomDecorator::AT_CD2) {
    if (residue_type == core::ResidueDecorator::LEU) return CH3;
    if (residue_type == core::ResidueDecorator::PHE ||
        residue_type == core::ResidueDecorator::HIS ||
        residue_type == core::ResidueDecorator::TYR) return CH;
    return C;
  }

  // CE
  if (atom_type == core::AtomDecorator::AT_CE) {
    if (residue_type == core::ResidueDecorator::LYS) return CH2;
    if (residue_type == core::ResidueDecorator::MET) return CH3;
    return C;
  }

  // CE1
  if (atom_type == core::AtomDecorator::AT_CE1) {
    if (residue_type == core::ResidueDecorator::PHE ||
        residue_type == core::ResidueDecorator::HIS ||
        residue_type == core::ResidueDecorator::TYR) return CH;
    return C;
  }

  // CE2
  if (atom_type == core::AtomDecorator::AT_CE2) {
    if (residue_type == core::ResidueDecorator::PHE ||
        residue_type == core::ResidueDecorator::TYR) return CH;
    return C;
  }
  // CZ
  if (atom_type == core::AtomDecorator::AT_CZ) {
    if (residue_type == core::ResidueDecorator::PHE) return CH;
    return C;
  }
//   // CZ1
//   if(atom_type == core::AtomDecorator::AT_CZ1) return C;

  // CZ2, CZ3, CE3
  if (atom_type == core::AtomDecorator::AT_CZ2 ||
      atom_type == core::AtomDecorator::AT_CZ3 ||
      atom_type == core::AtomDecorator::AT_CE3) {
    if (residue_type == core::ResidueDecorator::TRP) return CH;
    return C;
  }

  std::cerr << "Error in get_carbon_atom_type: atom not found "
      << atom_type << " " << residue_type << std::endl;
  return C;
}

FormFactorTable::FormFactorAtomType FormFactorTable::get_nitrogen_atom_type(
                     const core::AtomType& atom_type,
                     const core::ResidueType& residue_type) const {
  // N
  if (atom_type == core::AtomDecorator::AT_N) {
    if (residue_type == core::ResidueDecorator::PRO) return N;
    return NH;
  }
  //// ND
  //if(atom_type == core::AtomDecorator::AT_ND) return N;
  // ND1
  if (atom_type == core::AtomDecorator::AT_ND1) {
    if (residue_type == core::ResidueDecorator::HIS) return NH;
    return N;
  }
  // ND2
  if (atom_type == core::AtomDecorator::AT_ND2) {
    if (residue_type == core::ResidueDecorator::ASN) return NH2;
    return N;
  }
  // NH1, NH2
  if (atom_type == core::AtomDecorator::AT_NH1 ||
      atom_type == core::AtomDecorator::AT_NH2) {
    if (residue_type == core::ResidueDecorator::ARG) return NH2;
    return N;
  }
  // NE
  if (atom_type == core::AtomDecorator::AT_NE) {
    if (residue_type == core::ResidueDecorator::ARG) return NH;
    return N;
  }
  // NE1
  if (atom_type == core::AtomDecorator::AT_NE1) {
    if (residue_type == core::ResidueDecorator::TRP) return NH;
    return N;
  }
  // NE2
  if (atom_type == core::AtomDecorator::AT_NE2) {
    if (residue_type == core::ResidueDecorator::GLN) return NH2;
    return N;
  }
  // NZ
  if (atom_type == core::AtomDecorator::AT_NZ) {
    if (residue_type == core::ResidueDecorator::LYS) return NH3;
    return N;
  }
  std::cerr << "Error in get_nitrogen_atom_type: atom not found "
      << atom_type << " " << residue_type << std::endl;
  return N;
}

FormFactorTable::FormFactorAtomType FormFactorTable::get_oxygen_atom_type(
                     const core::AtomType& atom_type,
                     const core::ResidueType& residue_type) const {

  // O OE1 OE2 OD1 OD2 O1A O2A OXT OT1 OT2
  if (atom_type == core::AtomDecorator::AT_O ||
      atom_type == core::AtomDecorator::AT_OE1 ||
      atom_type == core::AtomDecorator::AT_OE2 ||
      atom_type == core::AtomDecorator::AT_OD1 ||
      atom_type == core::AtomDecorator::AT_OD2 ||
      //atom_type == core::AtomDecorator::AT_O1A ||
      //atom_type == core::AtomDecorator::AT_O2A ||
      //atom_type == core::AtomDecorator::AT_OT1 ||
      //atom_type == core::AtomDecorator::AT_OT2 ||
      atom_type == core::AtomDecorator::AT_OXT) return O;
  // OG
  if (atom_type == core::AtomDecorator::AT_OG) {
    if (residue_type == core::ResidueDecorator::SER) return OH;
    return O;
  }
  // OG1
  if (atom_type == core::AtomDecorator::AT_OG1) {
    if (residue_type == core::ResidueDecorator::THR) return OH;
    return O;
  }
  // OH
  if (atom_type == core::AtomDecorator::AT_OH) {
    if (residue_type == core::ResidueDecorator::TYR) return OH;
    return O;
  }
  std::cerr << "Error in get_oxygen_atom_type: atom not found "
      << atom_type << " " << residue_type << std::endl;
  return O;
}

FormFactorTable::FormFactorAtomType FormFactorTable::get_sulfur_atom_type(
                     const core::AtomType& atom_type,
                     const core::ResidueType& residue_type) const {
  // SD
  if (atom_type == core::AtomDecorator::AT_SD) return S;
  // SG
  if (atom_type == core::AtomDecorator::AT_SG) {
    if (residue_type == core::ResidueDecorator::CYS) return SH;
    return S;
  }
  std::cerr << "Error in get_sulfur_atom_type: atom not found "
      << atom_type << " " << residue_type << std::endl;
  return S;
}

FormFactorTable::FormFactorAtomType FormFactorTable::get_form_factor_atom_type(
                                   Particle * p, FormFactorType ff_type) const {
  core::ResidueType residue_type =
    core::get_residue_type(core::AtomDecorator::cast(p));
  core::AtomType atom_type =
      core::AtomDecorator::cast(p).get_type();
  String atom_name = atom_type.get_string();
  //    core::AtomDecorator::cast(p).get_pdb_atom_type().get_string();

  FormFactorAtomType ret_type = UNK;
  int name_start = 0;
  if (isdigit(atom_name[0]))
    name_start = 1;
  for (unsigned int i = 0; i < ALL_ATOM_SIZE; i++) {
    int comp_length = std::min(atom_name.length(), element_names[i].length());
    if (atom_name.substr(name_start, comp_length) == element_names[i]) {
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

const Floats & FormFactorTable::get_form_factors(Particle * p,
                                            FormFactorType ff_type) const {
  FormFactorAtomType ff_atom_type = get_form_factor_atom_type(p, ff_type);
  return form_factors_[(int)ff_atom_type];
}

IMPSAXS_END_NAMESPACE
