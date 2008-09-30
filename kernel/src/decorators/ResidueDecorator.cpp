/**
 *  \file AtomDecorator.cpp   \brief Simple atoms decorator.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/base_types.h"
#include "IMP/decorators/ResidueDecorator.h"
#include "IMP/decorators/AtomDecorator.h"
#include "IMP/log.h"

#include <sstream>
#include <vector>

IMP_BEGIN_NAMESPACE

IntKey ResidueDecorator::type_key_;
IntKey ResidueDecorator::index_key_;

#define TYPE_INIT(STR) STR= ResidueType(#STR);
#define TYPE_DEF(STR) ResidueType ResidueDecorator::STR;
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
TYPE_DEF(THY)

void ResidueDecorator::show(std::ostream &out, std::string pre) const
{
  out << pre << "residue #" << get_index() << " of type "
  << get_type() << std::endl;
}



IMP_DECORATOR_INITIALIZE(ResidueDecorator, DecoratorBase,
                         { type_key_= IntKey("residue type");
                           index_key_= IntKey("residue index");
                           TYPE_INIT(UNK);
                           TYPE_INIT(GLY);
                           TYPE_INIT(ALA);
                           TYPE_INIT(VAL);
                           TYPE_INIT(LEU);
                           TYPE_INIT(ILE);
                           TYPE_INIT(SER);
                           TYPE_INIT(THR);
                           TYPE_INIT(CYS);
                           TYPE_INIT(MET);
                           TYPE_INIT(PRO);
                           TYPE_INIT(ASP);
                           TYPE_INIT(ASN);
                           TYPE_INIT(GLU);
                           TYPE_INIT(GLN);
                           TYPE_INIT(LYS);
                           TYPE_INIT(ARG);
                           TYPE_INIT(HIS);
                           TYPE_INIT(PHE);
                           TYPE_INIT(TYR);
                           TYPE_INIT(TRP);
                           TYPE_INIT(ACE);
                           TYPE_INIT(NH2);
                           TYPE_INIT(ADE);
                           TYPE_INIT(URA);
                           TYPE_INIT(CYT);
                           TYPE_INIT(GUA);
                           TYPE_INIT(THY);
                         })

IMP_END_NAMESPACE
