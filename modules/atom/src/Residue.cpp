/**
 *  \file Atom.cpp   \brief Simple atoms decorator.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
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

#define NAME_DEF(STR) const ResidueType STR(ResidueType::add_key(#STR))
#define NAME_DEF2(NAME, STR) const ResidueType NAME(ResidueType::add_key(STR))
#define NAME_ALIAS(OLD_NAME, NAME, STRING) const ResidueType NAME       \
  (ResidueType::add_alias(OLD_NAME, STRING))

NAME_DEF(UNK);
NAME_DEF(GLY);
NAME_DEF(ALA);
NAME_DEF(VAL);
NAME_DEF(LEU);
NAME_DEF(ILE);
NAME_DEF(SER);
NAME_DEF(THR);
NAME_DEF(CYS);
NAME_DEF(MET);
NAME_DEF(PRO);
NAME_DEF(ASP);
NAME_DEF(ASN);
NAME_DEF(GLU);
NAME_DEF(GLN);
NAME_DEF(LYS);
NAME_DEF(ARG);
NAME_DEF(HIS);
NAME_DEF(PHE);
NAME_DEF(TYR);
NAME_DEF(TRP);
NAME_DEF(ACE);
NAME_DEF(NH2);

// RNA/DNA
NAME_DEF2(ADE, "A");
NAME_DEF2(URA, "U");
NAME_DEF2(CYT, "C");
NAME_DEF2(GUA, "G");
NAME_DEF2(THY, "T");
NAME_DEF2(DADE, "DA");
NAME_DEF2(DURA, "DU");
NAME_DEF2(DCYT, "DC");
NAME_DEF2(DGUA, "DG");
NAME_DEF2(DTHY, "DT");
// old format support
// the second parameter is a dummy name for macro only
NAME_ALIAS(ADE, ADE_A, "ADE");
NAME_ALIAS(URA, ADE_U, "URA");
NAME_ALIAS(CYT, ADE_C, "CYT");
NAME_ALIAS(GUA, ADE_G, "GUA");
NAME_ALIAS(THY, ADE_T, "THY");


ResidueType residue_name_from_pdb_string(std::string nm) {
  if (!ResidueType::get_key_exists(nm)) {
    std::ostringstream oss;
    oss<< "ResidueType " << nm << " does not exist.";
    throw ValueException(oss.str().c_str());
  }
  return ResidueType(nm.c_str());
}

void Residue::show(std::ostream &out) const
{
  out << "residue #" << get_index() << " of name "
      << get_residue_type();
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

Chain get_chain(Residue rd) {
  Hierarchy mhd(rd.get_particle());
  do {
    mhd= mhd.get_parent();
    if (mhd == Hierarchy()) {
      throw InvalidStateException("Residue is not the child of a chain");
    }
  } while (mhd.get_type() != Hierarchy::CHAIN &&
           mhd.get_type() != Hierarchy::NUCLEOTIDE &&
           mhd.get_type() != Hierarchy::MOLECULE);
  Chain cd(mhd.get_particle());
  return cd;
}

IMPATOM_END_NAMESPACE
