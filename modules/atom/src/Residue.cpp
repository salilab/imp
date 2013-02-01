/**
 *  \file Atom.cpp   \brief Simple atoms decorator.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/Residue.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Chain.h>

#include <IMP/base_types.h>
#include <IMP/log.h>

#include <sstream>
#include <vector>

IMPATOM_BEGIN_NAMESPACE

#define RNAME_DEF(STR) const ResidueType STR(ResidueType::add_key(#STR))
#define RNAME_DEF2(NAME, STR) const ResidueType NAME(ResidueType::add_key(STR))
#define RNAME_ALIAS(OLD_NAME, NAME, STRING) const ResidueType NAME       \
  (ResidueType::add_alias(OLD_NAME, STRING))

RNAME_DEF(UNK);
RNAME_DEF(GLY);
RNAME_DEF(ALA);
RNAME_DEF(VAL);
RNAME_DEF(LEU);
RNAME_DEF(ILE);
RNAME_DEF(SER);
RNAME_DEF(THR);
RNAME_DEF(CYS);
RNAME_DEF(MET);
RNAME_DEF(PRO);
RNAME_DEF(ASP);
RNAME_DEF(ASN);
RNAME_DEF(GLU);
RNAME_DEF(GLN);
RNAME_DEF(LYS);
RNAME_DEF(ARG);
RNAME_DEF(HIS);
RNAME_DEF(PHE);
RNAME_DEF(TYR);
RNAME_DEF(TRP);
RNAME_DEF(ACE);
RNAME_DEF(NH2);

// RNA/DNA
RNAME_DEF2(ADE, "A");
RNAME_DEF2(URA, "U");
RNAME_DEF2(CYT, "C");
RNAME_DEF2(GUA, "G");
RNAME_DEF2(THY, "T");
RNAME_DEF2(DADE, "DA");
RNAME_DEF2(DURA, "DU");
RNAME_DEF2(DCYT, "DC");
RNAME_DEF2(DGUA, "DG");
RNAME_DEF2(DTHY, "DT");
// old format support
// the second parameter is a dummy name for macro only
RNAME_ALIAS(ADE, ADE_A, "ADE");
RNAME_ALIAS(URA, ADE_U, "URA");
RNAME_ALIAS(CYT, ADE_C, "CYT");
RNAME_ALIAS(GUA, ADE_G, "GUA");
RNAME_ALIAS(THY, ADE_T, "THY");
RNAME_ALIAS(DADE, DADE_A, "DADE");
RNAME_ALIAS(DURA, DADE_U, "DURA");
RNAME_ALIAS(DCYT, DADE_C, "DCYT");
RNAME_ALIAS(DGUA, DADE_G, "DGUA");
RNAME_ALIAS(DTHY, DADE_T, "DTHY");

RNAME_DEF(HOH);
RNAME_DEF(HEME);
RNAME_ALIAS(HEME, HEME_OLD, "HEM");

void Residue::show(std::ostream &out) const
{
  out << "residue #" << get_index() << " of name "
      << get_residue_type();
}


void Residue::set_residue_type(ResidueType t)
{
  get_particle()->set_value(get_residue_type_key(), t.get_index());
}

IntKey Residue::get_index_key() {
  static IntKey k("residue_index");
  return k;
}

IntKey Residue::get_residue_type_key() {
  static IntKey k("residue_type");
  return k;
}

IntKey Residue::get_insertion_code_key() {
  static IntKey k("residue_icode");
  return k;
}


Hierarchy get_next_residue(Residue rd) {
  // only handle simple case so far
  Hierarchy p= rd.get_parent();
  /*if (!p.get_as_chain()) {
    IMP_NOT_IMPLEMENTED("get_next_residue() only handles the simple case"
                        << " so far. Complain about it.");
                        }*/
  Chain c= p.get_as_chain();
  IMP_USAGE_CHECK(c, "Parent of residue must be a chain. It is not.");
  Hierarchy r=get_residue(c, rd.get_index()+1);
  return r;
}

Hierarchy get_previous_residue(Residue rd) {
  // only handle simple case so far
  Hierarchy p= rd.get_parent();
  Chain c= p.get_as_chain();
  Hierarchy r=get_residue(c, rd.get_index()-1);
  return r;
}

namespace {
  struct RP: public std::pair<char, ResidueType> {
    RP(ResidueType rt, char c): std::pair<char, ResidueType>(c, rt) {}
  };
  const RP rp_names[]={RP(ALA, 'A'),
                    RP(ARG, 'R'),
                    RP(ASP, 'D'),
                    RP(ASN, 'N'),
                    RP(CYS, 'C'),
                    RP(GLN, 'Q'),
                    RP(GLU, 'E'),
                    RP(GLY, 'G'),
                    RP(HIS, 'H'),
                    RP(ILE, 'I'),
                    RP(LEU, 'L'),
                    RP(LYS, 'K'),
                    RP(MET, 'M'),
                    RP(PHE, 'F'),
                    RP(PRO, 'P'),
                    RP(SER, 'S'),
                    RP(THR, 'T'),
                    RP(TYR, 'Y'),
                    RP(TRP, 'W'),
                    RP(VAL, 'V'),
                    RP(UNK, 'X')};
  const IMP::base::map<char, ResidueType> rp_map(rp_names,
                                        rp_names+sizeof(rp_names)/sizeof(RP));
}

ResidueType get_residue_type(char c) {
  if (rp_map.find(c) == rp_map.end()) {
    IMP_THROW("Residue name not found " << c, ValueException);
  } else {
    return rp_map.find(c)->second;
  }
}

char get_one_letter_code(ResidueType c) {
  for (unsigned int i=0; i< rp_map.size(); ++i) {
    if (rp_names[i].second==c) return rp_names[i].first;
  }
  return 'X';
}

namespace {
bool check_residue(Particle*p) {
  unsigned int t=p->get_value(Residue::get_residue_type_key());
  if (t >= ResidueType::get_number_unique()) {
    IMP_THROW("Invalid residue type: " << t,
              ValueException);
  }
  return true;
}
}

IMP_CHECK_DECORATOR(Residue, check_residue);


IMPATOM_END_NAMESPACE
