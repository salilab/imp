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
TYPE_DEF(ADE);
TYPE_DEF(URA);
TYPE_DEF(CYT);
TYPE_DEF(GUA);
TYPE_DEF(THY);
TYPE_DEF(DADE);
TYPE_DEF(DURA);
TYPE_DEF(DCYT);
TYPE_DEF(DGUA);
TYPE_DEF(DTHY);

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
  MolecularHierarchy mhd(rd.get_particle());
  do {
    mhd= mhd.get_parent();
    if (mhd == MolecularHierarchy()) {
      throw InvalidStateException("Residue is not the child of a chain");
    }
  } while (mhd.get_type() != MolecularHierarchy::CHAIN);
  Chain cd(mhd.get_particle());
  return cd.get_id();
}

IMPATOM_END_NAMESPACE
