/**
 *  \file AtomDecorator.cpp   \brief Simple atoms decorator.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/ResidueDecorator.h>
#include <IMP/core/AtomDecorator.h>

#include <IMP/base_types.h>
#include <IMP/log.h>

#include <sstream>
#include <vector>

IMPCORE_BEGIN_NAMESPACE

#define TYPE_DEF(STR) ResidueType ResidueDecorator::STR(#STR);
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

ResidueType residue_type_from_pdb_string(std::string nm) {
  if (!ResidueType::get_key_exists(nm)) {
    std::ostringstream oss;
    oss<< "ResidueType " << nm << " does not exist.";
    throw ValueException(oss.str().c_str());
  }
  return ResidueType(nm.c_str());
}

void ResidueDecorator::show(std::ostream &out, std::string pre) const
{
  out << pre << "residue #" << get_index() << " of type "
  << get_type() << std::endl;
}

IntKey ResidueDecorator::get_index_key() {
  IntKey k("residue_index");
  return k;
}

IntKey ResidueDecorator::get_type_key() {
  IntKey k("residue_type");
  return k;
}
IMPCORE_END_NAMESPACE
