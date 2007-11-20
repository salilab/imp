/**
 *  \file AtomDecorator.cpp   \brief Simple atoms decorator.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <IMP/decorators/ResidueDecorator.h>
#include <IMP/decorators/AtomDecorator.h>
#include <sstream>
#include <IMP/log.h>
#include <vector>

namespace IMP
{



IntKey ResidueDecorator::type_key_;
IntKey ResidueDecorator::index_key_;

#define TYPE(str) ResidueType ResidueDecorator::str(#str);
TYPE(UNK);
TYPE(GLY);
TYPE(ALA);
TYPE(VAL);
TYPE(LEU);
TYPE(ILE);
TYPE(SER);
TYPE(THR);
TYPE(CYS);
TYPE(MET);
TYPE(PRO);
TYPE(ASP);
TYPE(ASN);
TYPE(GLU);
TYPE(GLN);
TYPE(LYS);
TYPE(ARG);
TYPE(HIS);
TYPE(PHE);
TYPE(TYR);
TYPE(TRP);
TYPE(ACE);
TYPE(NH2);
TYPE(ADE);
TYPE(URA);
TYPE(CYT);
TYPE(GUA);
TYPE(THY)

void ResidueDecorator::show(std::ostream &out, std::string pre) const
{
  out << pre << "residue #" << get_index() << " of type "
  << get_type() << std::endl;
}



IMP_DECORATOR_INITIALIZE(ResidueDecorator, DecoratorBase,
                         { type_key_= IntKey("residue type");
                             index_key_= IntKey("residue index");
                         })


} // namespace IMP
