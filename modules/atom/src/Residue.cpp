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

#define TYPE_DEF(STR) const ResidueType STR(#STR);
#define TYPE_DEF2(NAME, STR) const ResidueType NAME(STR)
#define TYPE_ALIAS(OLD_NAME, NAME, STRING) const ResidueType NAME       \
  (ResidueType::add_alias(OLD_NAME, STRING))

TYPE_DEF(UNK);
TYPE_DEF(GLY);
TYPE_DEF(ALA);
TYPE_DEF(VAL);
TYPE_DEF(LEU);
TYPE_DEF(ILE);
TYPE_DEF(SER);
TYPE_DEF(THR);
TYPE_DEF(CYS);
TYPE_DEF(MET);
TYPE_DEF(PRO);
TYPE_DEF(ASP);
TYPE_DEF(ASN);
TYPE_DEF(GLU);
TYPE_DEF(GLN);
TYPE_DEF(LYS);
TYPE_DEF(ARG);
TYPE_DEF(HIS);
TYPE_DEF(PHE);
TYPE_DEF(TYR);
TYPE_DEF(TRP);
TYPE_DEF(ACE);
TYPE_DEF(NH2);
// RNA/DNA
TYPE_DEF2(ADE, "A");
TYPE_DEF2(URA, "U");
TYPE_DEF2(CYT, "C");
TYPE_DEF2(GUA, "G");
TYPE_DEF2(THY, "T");
TYPE_DEF2(DADE, "DA");
TYPE_DEF2(DURA, "DU");
TYPE_DEF2(DCYT, "DC");
TYPE_DEF2(DGUA, "DG");
TYPE_DEF2(DTHY, "DT");
// old format support
// the second parameter is a dummy name for macro only
TYPE_ALIAS(ADE, ADE_A, "ADE");
TYPE_ALIAS(URA, ADE_U, "URA");
TYPE_ALIAS(CYT, ADE_C, "CYT");
TYPE_ALIAS(GUA, ADE_G, "GUA");
TYPE_ALIAS(THY, ADE_T, "THY");


ResidueType residue_type_from_pdb_string(std::string nm) {
  if (!ResidueType::get_key_exists(nm)) {
    std::ostringstream oss;
    oss<< "ResidueType " << nm << " does not exist.";
    throw ValueException(oss.str().c_str());
  }
  return ResidueType(nm.c_str());
}

void Residue::show(std::ostream &out, std::string pre) const
{
  out << pre << "residue #" << get_index() << " of type "
      << get_residue_type() << std::endl;
}

IntKey Residue::get_index_key() {
  static IntKey k("residue_index");
  return k;
}

IntKey Residue::get_type_key() {
  static IntKey k("residue_type");
  return k;
}

IntKey Residue::get_insertion_code_key() {
  static IntKey k("residue_icode");
  return k;
}

char get_chain(Residue rd) {
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
  return cd.get_id();
}

IMPATOM_END_NAMESPACE
