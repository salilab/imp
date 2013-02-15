/**
 *  \file FormFactorTable.h   \brief A class for computation of
 * atomic and residue level form factors for SAXS calculations
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/saxs/FormFactorTable.h>
#include <IMP/atom/Atom.h>
#include <IMP/constants.h>
#include <IMP/algebra/utility.h>

#include <fstream>
#include <algorithm>
#include <cmath>

IMPSAXS_BEGIN_NAMESPACE

IntKey FormFactorTable::form_factor_type_key_ = IntKey("form factor key");

std::map<atom::Element, FormFactorTable::FormFactorAtomType>
FormFactorTable::element_ff_type_map_;

std::map<atom::ResidueType, FormFactorTable::FormFactor>
FormFactorTable::residue_type_form_factor_map_;

Float FormFactorTable::zero_form_factors_[] = {
  -0.720147, -0.720228,
   //   H       He
  1.591, 2.591, 3.591, 0.50824, 6.16294, 4.94998, 7.591, 6.993,
  // Li     Be      B     C       N        O       F      Ne
  7.9864, 8.9805, 9.984, 10.984, 13.0855, 9.36656, 13.984, 16.591,
  //  Na      Mg     Al     Si      P        S       Cl     Ar
  15.984, 14.9965, 20.984, 21.984, 20.9946, 23.984,
  // K     Ca2+     Cr      Mn      Fe2+      Co
  24.984, 25.984, 24.9936, 30.9825, 31.984, 49.16,
  // Ni   Cu      Zn2+       Se      Br      I
  70.35676, 71.35676, 72.324,  73.35676,
  // Ir         Pt      Au      Hg
  -0.211907, -0.932054, -1.6522, 5.44279, 4.72265,4.0025,4.22983,3.50968,8.64641
  //  CH        CH2        CH3     NH       NH2       NH3     OH       OH2   SH
};

Float FormFactorTable::vacuum_zero_form_factors_[] = {
  0.999953, 0.999872, 2.99, 3.99, 4.99, 5.9992, 6.9946, 7.9994, 8.99, 9.999,
  //   H       He      Li     Be     B    C       N       O      F     Ne
  10.9924, 11.9865, 12.99, 13.99, 14.9993, 15.9998, 16.99, 17.99,
  //  Na      Mg     Al     Si      P        S       Cl     Ar
  18.99, 18.0025,  23.99, 24.99,  24.0006, 26.99,
  // K     Ca2+     Cr     Mn      Fe2+      Co
  27.99, 28.99, 27.9996, 33.99, 34.99, 52.99, 76.99, 77.99, 78.9572, 79.99,
  // Ni   Cu      Zn2+    Se     Br     I       Ir     Pt      Au     Hg
  6.99915, 7.99911, 8.99906, 7.99455, 8.99451, 9.99446, 8.99935, 9.9993, 16.9998
  //  CH      CH2     CH3     NH       NH2       NH3     OH      OH2      SH
};

Float FormFactorTable::dummy_zero_form_factors_[] = {
  1.7201, 1.7201, 1.399, 1.399, 1.399 , 5.49096, 0.83166, 3.04942, 1.399, 3.006,
  //  H     He     Li?    Be?    B?       C        N        O      F?     Ne
  3.006, 3.006, 3.006, 3.006, 1.91382, 6.63324, 3.006, 1.399,
  // Na     Mg    Al?    Si?      P        S      Cl?    Ar?
  3.006, 3.006, 3.006, 3.006, 3.006, 3.006,
  // K?   Ca2+    Cr?    Mn?   Fe2+   Co?
  3.006, 3.006, 3.006, 3.006, 3.006, 3.83, 6.63324, 6.63324, 6.63324, 6.63324,
  // Ni?   Cu?   Zn2+    Se     Br?     I?   Ir?      Pt?       Au      Hg
  7.21106, 8.93116, 10.6513, 2.55176, 4.27186, 5.99196, 4.76952, 6.48962,8.35334
  //  CH       CH2      CH3     NH       NH2       NH3     OH       OH2   SH
};

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

FormFactorTable::FormFactorTable() {
  init_element_form_factor_map();
  init_residue_type_form_factor_map();
}

FormFactorTable::FormFactorTable(const String& table_name, Float min_q,
                                 Float max_q, Float delta_q):
  min_q_(min_q), max_q_(max_q), delta_q_(delta_q)
{
  init_element_form_factor_map();
  init_residue_type_form_factor_map();

  // read form factor coefficients from file
  int ffnum = read_form_factor_table(table_name);

  if(ffnum > 0) { // form factors found in form factor file
    // init zero_form_factors so that they are computed from  the file
    for(int i=0; i<HEAVY_ATOM_SIZE; i++) {
      zero_form_factors_[i] = 0.0;
      vacuum_zero_form_factors_[i] = 0.0;
      dummy_zero_form_factors_[i] = 0.0;
    }
    // init all the tables
    unsigned int number_of_q_entries =
      algebra::get_rounded((max_q_ - min_q_) / delta_q_ ) + 1;
    Floats form_factor_template(number_of_q_entries, 0.0);
    form_factors_ = std::vector<Floats> (HEAVY_ATOM_SIZE, form_factor_template);
    vacuum_form_factors_ =
      std::vector<Floats> (HEAVY_ATOM_SIZE, form_factor_template);
    dummy_form_factors_ =
      std::vector<Floats> (HEAVY_ATOM_SIZE, form_factor_template);

    // compute all the form factors
    compute_form_factors_all_atoms();
    compute_form_factors_heavy_atoms();
  }
}

void FormFactorTable::init_element_form_factor_map() {
  element_ff_type_map_[atom::H] = H;
  element_ff_type_map_[atom::He] = He;
  element_ff_type_map_[atom::Li] = Li;
  element_ff_type_map_[atom::Be] = Be;
  element_ff_type_map_[atom::B] = B;
  element_ff_type_map_[atom::C] = C;
  element_ff_type_map_[atom::N] = N;
  element_ff_type_map_[atom::O] = O;
  element_ff_type_map_[atom::F] = F;
  element_ff_type_map_[atom::Ne] = Ne;
  element_ff_type_map_[atom::Na] = Na;
  element_ff_type_map_[atom::Mg] = Mg;
  element_ff_type_map_[atom::Al] = Al;
  element_ff_type_map_[atom::Si] = Si;
  element_ff_type_map_[atom::P] = P;
  element_ff_type_map_[atom::S] = S;
  element_ff_type_map_[atom::Cl] = Cl;
  element_ff_type_map_[atom::Ar] = Ar;
  element_ff_type_map_[atom::K] = K;
  element_ff_type_map_[atom::Ca] = Ca;
  element_ff_type_map_[atom::Cr] = Cr;
  element_ff_type_map_[atom::Mn] = Mn;
  element_ff_type_map_[atom::Fe] = Fe;
  element_ff_type_map_[atom::Co] = Co;
  element_ff_type_map_[atom::Ni] = Ni;
  element_ff_type_map_[atom::Cu] = Cu;
  element_ff_type_map_[atom::Zn] = Zn;
  element_ff_type_map_[atom::Se] = Se;
  element_ff_type_map_[atom::Br] = Br;
  element_ff_type_map_[atom::I] = I;
  element_ff_type_map_[atom::Ir] = Ir;
  element_ff_type_map_[atom::Pt] = Pt;
  element_ff_type_map_[atom::Au] = Au;
  element_ff_type_map_[atom::Hg] = Hg;
}

void FormFactorTable::init_residue_type_form_factor_map() {
  residue_type_form_factor_map_[atom::ALA] = FormFactor(9.037, 37.991, 28.954);
  residue_type_form_factor_map_[atom::ARG] = FormFactor(23.289, 84.972, 61.683);
  residue_type_form_factor_map_[atom::ASP] = FormFactor(20.165, 58.989, 38.824);
  residue_type_form_factor_map_[atom::ASN] = FormFactor(19.938, 59.985, 40.047);
  residue_type_form_factor_map_[atom::CYS] = FormFactor(18.403, 53.991, 35.588);
  residue_type_form_factor_map_[atom::GLN] = FormFactor(19.006, 67.984, 48.978);
  residue_type_form_factor_map_[atom::GLU] = FormFactor(19.233, 66.989, 47.755);
  residue_type_form_factor_map_[atom::GLY] = FormFactor(10.689, 28.992, 18.303);
  residue_type_form_factor_map_[atom::HIS] = FormFactor(21.235, 78.977, 57.742);
  residue_type_form_factor_map_[atom::ILE] = FormFactor(6.241, 61.989, 55.748);
  residue_type_form_factor_map_[atom::LEU] = FormFactor(6.241, 61.989, 55.748);
  residue_type_form_factor_map_[atom::LYS] = FormFactor(10.963, 70.983, 60.020);
  residue_type_form_factor_map_[atom::MET] = FormFactor(16.539, 69.989, 53.450);
  residue_type_form_factor_map_[atom::PHE] = FormFactor(9.206, 77.986, 68.7806);
  residue_type_form_factor_map_[atom::PRO] = FormFactor(8.613, 51.9897, 43.377);
  residue_type_form_factor_map_[atom::SER] = FormFactor(13.987, 45.991, 32.004);
  residue_type_form_factor_map_[atom::THR] = FormFactor(13.055, 53.99, 40.935);
  residue_type_form_factor_map_[atom::TYR] = FormFactor(14.156, 85.986, 71.83);
  residue_type_form_factor_map_[atom::TRP] = FormFactor(14.945, 98.979, 84.034);
  residue_type_form_factor_map_[atom::VAL] = FormFactor(7.173, 53.9896, 46.817);
  residue_type_form_factor_map_[atom::UNK] = FormFactor(9.037, 37.991, 28.954);
}

int FormFactorTable::read_form_factor_table(const String & table_name)
{
  std::ifstream s(table_name.c_str());
  if (!s) {
    IMP_THROW("Can't find form factor table file " << table_name,
              IOException);
  }

  atom::ElementTable e_table = atom::get_element_table();

  // init coefficients table
  form_factors_coefficients_ =
    std::vector<AtomFactorCoefficients>(ALL_ATOM_SIZE);

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
  int counter = 0;
  while (!s.eof()) {
    s >> coeff;
    // find FormFactorAtomType
    atom::Element e = e_table.get_element(coeff.atom_type_);
    FormFactorAtomType ff_type = get_form_factor_atom_type(e);
    if(ff_type != UNK) {
      form_factors_coefficients_[ff_type] = coeff;
      counter++;
      IMP_LOG_TERSE( "read_form_factor_table: Atom type found: " <<
              coeff.atom_type_ << std::endl);
    } else {
      IMP_LOG_TERSE( "Atom type is not supported "
              << coeff.atom_type_ << std::endl);
    }
  }
  IMP_LOG_TERSE( counter << " form factors were read from file " << std::endl);
  return counter;
}

void FormFactorTable::show(std::ostream & out, std::string prefix) const
{
  for (unsigned int i = 0; i < HEAVY_ATOM_SIZE; i++) {
    out << prefix << " FFATOMTYPE " << i << " zero_ff " << zero_form_factors_[i]
        << " vacuum_ff "  << vacuum_zero_form_factors_[i]
        << " dummy_ff " << dummy_zero_form_factors_[i] << std::endl;
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
  int number_of_q_entries = (int)std::ceil((max_q_ - min_q_) / delta_q_ );

  // iterate over different atom types
  for (unsigned int i = 0; i < ALL_ATOM_SIZE; i++) {
    // form factors for all the q range
    // volr_coeff = - v_i^(2/3) / 4PI
    Float volr_coeff =
      - std::pow(form_factors_coefficients_[i].excl_vol_, (2.0/3.0)) / (16*PI);

    // iterate over q
    for (int iq = 0; iq < number_of_q_entries; iq++) {
      Float q = min_q_ + (Float)iq * delta_q_;
      Float s = q/(4*PI);

      // c
      vacuum_form_factors_[i][iq] = form_factors_coefficients_[i].c_;

      // SUM [a_i * EXP( - b_i * (q/4pi)^2 )] Waasmaier and Kirfel (1995)
      for (unsigned int j = 0; j < 5; j++) {
        vacuum_form_factors_[i][iq] += form_factors_coefficients_[i].a_[j]
                   * std::exp(-form_factors_coefficients_[i].b_[j] * s * s);
      }
      // subtract solvation: rho * v_i * EXP( (- v_i^(2/3) / (4pi)) * q^2  )
      dummy_form_factors_[i][iq] =
        rho_ * form_factors_coefficients_[i].excl_vol_
             * std::exp(volr_coeff * q * q);

      form_factors_[i][iq] =
        vacuum_form_factors_[i][iq] - dummy_form_factors_[i][iq];
    }

    // zero form factors
    zero_form_factors_[i] = form_factors_coefficients_[i].c_;
    for (unsigned int j = 0; j < 5; j++) {
      zero_form_factors_[i] += form_factors_coefficients_[i].a_[j];
    }
    vacuum_zero_form_factors_[i] = zero_form_factors_[i];
    dummy_zero_form_factors_[i] = rho_* form_factors_coefficients_[i].excl_vol_;
    // subtract solvation
    zero_form_factors_[i] -= rho_ * form_factors_coefficients_[i].excl_vol_;
  }
}

void FormFactorTable::compute_form_factors_heavy_atoms()
{
  int number_of_q_entries = (int)std::ceil((max_q_ - min_q_) / delta_q_ );
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
    case OH2:
      element_type = O;
      h_num = 2;
      break;
    case SH:
      element_type = S;
      h_num = 1;
      break;
    default:
      break;
    }

    // full form factors
    for(int iq = 0; iq < number_of_q_entries; iq++) {
      // ff(i) = ff(element) + h_num*ff(hydrogen)
      form_factors_[i][iq] = form_factors_[element_type][iq] +
        h_num * form_factors_[H][iq];
      vacuum_form_factors_[i][iq] = vacuum_form_factors_[element_type][iq] +
        h_num * vacuum_form_factors_[H][iq];
      dummy_form_factors_[i][iq] = dummy_form_factors_[element_type][iq] +
        h_num * dummy_form_factors_[H][iq];
    }

    // zero form factors
    zero_form_factors_[i] = zero_form_factors_[element_type] +
      h_num * zero_form_factors_[H];
    vacuum_zero_form_factors_[i] = vacuum_zero_form_factors_[element_type] +
      h_num * vacuum_zero_form_factors_[H];
    dummy_zero_form_factors_[i] = dummy_zero_form_factors_[element_type] +
      h_num * dummy_zero_form_factors_[H];
  }
}

FormFactorTable::FormFactorAtomType FormFactorTable::get_carbon_atom_type(
                       const atom::AtomType& atom_type,
                       const atom::ResidueType& residue_type) const {
  // protein atoms
  // CH
  if (atom_type == atom::AT_CH) return CH;
  // CH2
  if (atom_type == atom::AT_CH2) return CH2;
  // CH3
  if (atom_type == atom::AT_CH3) return CH3;
  // C
  if (atom_type == atom::AT_C) return C;

  // CA
  if (atom_type == atom::AT_CA) {
    if (residue_type == atom::GLY) return CH2; // Glycine has 2 hydrogens
    return CH;
  }
  // CB
  if (atom_type == atom::AT_CB) {
    if (residue_type == atom::ILE || residue_type == atom::THR ||
        residue_type == atom::VAL) return CH;
    if (residue_type == atom::ALA) return CH3;
    return CH2;
  }
  // CG1
  if (atom_type == atom::AT_CG) {
    if (residue_type == atom::ASN || residue_type == atom::ASP ||
        residue_type == atom::HIS || residue_type == atom::PHE ||
        residue_type == atom::TRP || residue_type == atom::TYR) return C;
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
    if (residue_type == atom::GLU || residue_type == atom::GLN) return C;
    return CH2;
  }
  // CD1
  if (atom_type == atom::AT_CD1) {
    if (residue_type == atom::LEU || residue_type == atom::ILE) return CH3;
    if (residue_type == atom::PHE || residue_type == atom::TRP ||
        residue_type == atom::TYR) return CH;
    return C;
  }
  // CD2
  if (atom_type == atom::AT_CD2) {
    if (residue_type == atom::LEU) return CH3;
    if (residue_type == atom::PHE || residue_type == atom::HIS ||
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
    if (residue_type == atom::PHE || residue_type == atom::HIS ||
        residue_type == atom::TYR) return CH;
    return C;
  }
  // CE2
  if (atom_type == atom::AT_CE2) {
    if (residue_type == atom::PHE || residue_type == atom::TYR) return CH;
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

  // DNA/RNA atoms
  // C5'
  if (atom_type == atom::AT_C5p) return CH2;
  // C1', C2', C3', C4'
  if (atom_type == atom::AT_C4p || atom_type == atom::AT_C3p ||
      atom_type == atom::AT_C2p || atom_type == atom::AT_C1p) return CH;
  // C2
  if (atom_type == atom::AT_C2) {
    if(residue_type == atom::DADE || residue_type == atom::ADE) return CH;
    return C;
  }
  // C4
  if (atom_type == atom::AT_C4) return C;
  // C5
  if (atom_type == atom::AT_C5) {
    if(residue_type == atom::DCYT || residue_type == atom::CYT ||
       residue_type == atom::DURA || residue_type == atom::URA) return CH;
    return C;
  }
  // C6
  if (atom_type == atom::AT_C6) {
    if(residue_type == atom::DCYT || residue_type == atom::CYT ||
       residue_type == atom::DURA || residue_type == atom::URA ||
       residue_type == atom::DTHY || residue_type == atom::THY) return CH;
    return C;
  }
  // C7
  if (atom_type == atom::AT_C7) return CH3;
  // C8
  if (atom_type == atom::AT_C8) return CH;

  IMP_WARN_ONCE(atom_type.get_string()+residue_type.get_string(),
                "Carbon atom not found, using default C form factor for "
                << atom_type << " " << residue_type << std::endl,
                warn_context_);
  return C;
}

FormFactorTable::FormFactorAtomType FormFactorTable::get_nitrogen_atom_type(
                     const atom::AtomType& atom_type,
                     const atom::ResidueType& residue_type) const {
  // protein atoms
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
  if (atom_type == atom::AT_NH1 || atom_type == atom::AT_NH2) {
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

  // DNA/RNA atoms
  // N1
  if (atom_type == atom::AT_N1) {
    if(residue_type == atom::DGUA || residue_type == atom::GUA) return NH;
    return N;
  }
  // N2, N4, N6
  if (atom_type == atom::AT_N2 || atom_type == atom::AT_N4 ||
      atom_type == atom::AT_N6) return NH2;
  // N3
  if (atom_type == atom::AT_N3) {
    if(residue_type == atom::DURA || residue_type == atom::URA) return NH;
    return N;
  }
  // N7, N9
  if (atom_type == atom::AT_N7 || atom_type == atom::AT_N9) return N;

  IMP_WARN_ONCE(atom_type.get_string()+residue_type.get_string(),
                "Nitrogen atom not found, using default N form factor for "
                << atom_type << " " << residue_type << std::endl,
                warn_context_);
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

  // DNA/RNA atoms
  // O1P, O3', O2P, O2',O4',05', O2,O4,O6
  if (atom_type == atom::AT_OP1 || atom_type == atom::AT_O3p ||
      atom_type == atom::AT_OP2 || //atom_type == atom::AT_O2p ||
      atom_type == atom::AT_O4p || atom_type == atom::AT_O5p ||
      atom_type == atom::AT_O2 || atom_type == atom::AT_O4 ||
      atom_type == atom::AT_O6) return O;
  //  O2'
  if (atom_type == atom::AT_O2p)  return OH;

  // water molecule
  if(residue_type == atom::HOH) return OH2;

  IMP_WARN_ONCE(atom_type.get_string()+residue_type.get_string(),
                "Oxygen atom not found, using default O form factor for "
                << atom_type << " " << residue_type << std::endl,
                warn_context_);
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
  IMP_WARN_ONCE(atom_type.get_string()+residue_type.get_string(),
                "Sulfur atom not found, using default S form factor for "
                << atom_type << " " << residue_type << std::endl,
                warn_context_);
  return S;
}

FormFactorTable::FormFactorAtomType FormFactorTable::get_form_factor_atom_type(
                                   Particle *p, FormFactorType ff_type) const {
  atom::Atom ad = atom::Atom::decorate_particle(p);
  atom::ResidueType residue_type = atom::get_residue(ad).get_residue_type();
  atom::AtomType atom_type = ad.get_atom_type();

  // find FormFactorAtomType
  FormFactorAtomType ret_type =
    get_form_factor_atom_type((atom::Element)ad.get_element());

  if (ff_type == HEAVY_ATOMS) {
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

  if(ret_type >= HEAVY_ATOM_SIZE) {
    IMP_WARN( "Can't find form factor for particle "
              << atom::Atom(p).get_atom_type().get_string()
              << " using default value of nitrogen" << std::endl);
    ret_type = N;
  }
  return ret_type;
}

FormFactorTable::FormFactorAtomType FormFactorTable::get_form_factor_atom_type(
                                                       atom::Element e) const {
  std::map<atom::Element, FormFactorAtomType>::const_iterator i =
    element_ff_type_map_.find(e);
  if(i != element_ff_type_map_.end())
    return i->second;
  else
    return UNK;
}

float FormFactorTable::get_form_factor(atom::ResidueType rt) const {
  std::map<atom::ResidueType, FormFactor>::const_iterator i =
    residue_type_form_factor_map_.find(rt);
  if(i != residue_type_form_factor_map_.end())
    return i->second.ff_;
  else {
    IMP_WARN("Can't find form factor for residue " << rt.get_string()
             << " using default value of ALA " << std::endl);
    return residue_type_form_factor_map_.find(atom::UNK)->second.ff_;
  }
}

float FormFactorTable::get_vacuum_form_factor(atom::ResidueType rt) const {
  std::map<atom::ResidueType, FormFactor>::const_iterator i =
    residue_type_form_factor_map_.find(rt);
  if(i != residue_type_form_factor_map_.end())
    return i->second.vacuum_ff_;
  else {
    IMP_WARN("Can't find form factor for residue " << rt.get_string()
             << " using default value of ALA " << std::endl);
    return residue_type_form_factor_map_.find(atom::UNK)->second.vacuum_ff_;
  }
}

float FormFactorTable::get_dummy_form_factor(atom::ResidueType rt) const {
  std::map<atom::ResidueType, FormFactor>::const_iterator i =
    residue_type_form_factor_map_.find(rt);
  if(i != residue_type_form_factor_map_.end())
    return i->second.dummy_ff_;
  else {
    IMP_WARN("Can't find form factor for residue " << rt.get_string()
             << " using default value of ALA " << std::endl);
    return residue_type_form_factor_map_.find(atom::UNK)->second.dummy_ff_;
  }
}

Float FormFactorTable::get_form_factor(Particle *p,
                                       FormFactorType ff_type) const {
  if(ff_type == CA_ATOMS) { // residue level form factors
    atom::ResidueType residue_type =
      atom::get_residue(atom::Atom(p)).get_residue_type();
    return get_form_factor(residue_type);
  }

  // atomic form factor, initialization by request
  if (p->has_attribute(form_factor_type_key_)) {
    return zero_form_factors_[p->get_value(form_factor_type_key_)];
  }
  FormFactorAtomType ff_atom_type = get_form_factor_atom_type(p, ff_type);
  if(ff_atom_type >= HEAVY_ATOM_SIZE) {
    IMP_WARN( "Can't find form factor for particle "
              << atom::Atom(p).get_atom_type().get_string()
              << " using default" << std::endl);
    ff_atom_type = N;
  }
  Float form_factor = zero_form_factors_[ff_atom_type];
  p->add_cache_attribute(form_factor_type_key_, ff_atom_type);
  return form_factor;
}

Float FormFactorTable::get_vacuum_form_factor(Particle *p,
                                              FormFactorType ff_type) const {
  if(ff_type == CA_ATOMS) { // residue level form factors
    atom::ResidueType residue_type =
      atom::get_residue(atom::Atom(p)).get_residue_type();
    return get_vacuum_form_factor(residue_type);
  }

  if (p->has_attribute(form_factor_type_key_)) {
    return vacuum_zero_form_factors_[p->get_value(form_factor_type_key_)];
  }

  FormFactorAtomType ff_atom_type = get_form_factor_atom_type(p, ff_type);
  Float form_factor = vacuum_zero_form_factors_[ff_atom_type];
  p->add_attribute(form_factor_type_key_, ff_atom_type);
  return form_factor;
}

Float FormFactorTable::get_dummy_form_factor(Particle *p,
                                             FormFactorType ff_type) const {
  if(ff_type == CA_ATOMS) { // residue level form factors
    atom::ResidueType residue_type =
      atom::get_residue(atom::Atom(p)).get_residue_type();
    return get_dummy_form_factor(residue_type);
  }

  if (p->has_attribute(form_factor_type_key_)) {
    return dummy_zero_form_factors_[p->get_value(form_factor_type_key_)];
  }

  FormFactorAtomType ff_atom_type = get_form_factor_atom_type(p, ff_type);
  Float form_factor = dummy_zero_form_factors_[ff_atom_type];
  p->add_attribute(form_factor_type_key_, ff_atom_type);
  return form_factor;
}

Float FormFactorTable::get_radius(Particle* p, FormFactorType ff_type) const {
  // dummy_zero_form_factor = volume * rho
  // volume = 4/3 * pi * r^3
  // r^3 = 3*dummy_zero_form_factor / 4*pi*rho
  static Float one_third = 1.0/3;
  static Float c = 3.0/(4*PI*rho_);
  Float form_factor = get_dummy_form_factor(p, ff_type);
  return std::pow(c*form_factor, one_third);
}

Float FormFactorTable::get_volume(Particle* p, FormFactorType ff_type) const {
  // dummy_zero_form_factor = volume * rho
  Float form_factor = get_dummy_form_factor(p, ff_type);
  return form_factor/rho_;
}

const Floats& FormFactorTable::get_form_factors(Particle *p,
                                                FormFactorType ff_type) const {
  // initialization by request
  // store the index of the form factors in the particle
  if (p->has_attribute(form_factor_type_key_))
    return form_factors_[p->get_value(form_factor_type_key_)];

  FormFactorAtomType ff_atom_type = get_form_factor_atom_type(p, ff_type);
  p->add_attribute(form_factor_type_key_, ff_atom_type);
  return form_factors_[ff_atom_type];
}

const Floats& FormFactorTable::get_vacuum_form_factors(Particle *p,
                                                FormFactorType ff_type) const {
  // initialization by request
  // store the index of the form factors in the particle
  if (p->has_attribute(form_factor_type_key_))
    return vacuum_form_factors_[p->get_value(form_factor_type_key_)];

  FormFactorAtomType ff_atom_type = get_form_factor_atom_type(p, ff_type);
  p->add_attribute(form_factor_type_key_, ff_atom_type);
  return vacuum_form_factors_[ff_atom_type];
}

const Floats& FormFactorTable::get_dummy_form_factors(Particle *p,
                                                FormFactorType ff_type) const {
  // initialization by request
  // store the index of the form factors in the particle
  if (p->has_attribute(form_factor_type_key_))
    return dummy_form_factors_[p->get_value(form_factor_type_key_)];

  FormFactorAtomType ff_atom_type = get_form_factor_atom_type(p, ff_type);
  p->add_attribute(form_factor_type_key_, ff_atom_type);
  return dummy_form_factors_[ff_atom_type];
}

FormFactorTable* default_form_factor_table() {
  static FormFactorTable ff;
  return &ff;
}

IMPSAXS_END_NAMESPACE
