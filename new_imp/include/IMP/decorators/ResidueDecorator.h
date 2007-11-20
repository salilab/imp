/**
 *  \file ResidueDecorator.h     \brief A decorator for Residues.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_RESIDUE_DECORATOR_H
#define __IMP_RESIDUE_DECORATOR_H

#include "../Particle.h"
#include "../Model.h"
#include "utility.h"
#include "DecoratorBase.h"


namespace IMP
{



//! A decorator for a residue.
/**
   As with the AtomDecorator, the types of residues may been to be expanded
   dynamically. This can be easily done in an analogous manner when we
   need it. 
 */
  class IMPDLLEXPORT ResidueDecorator: public DecoratorBase
{
  IMP_DECORATOR(ResidueDecorator, DecoratorBase,
                return p->has_attribute(type_key_)
                       && p->has_attribute(index_key_),
                { p->add_attribute(type_key_, -1);
                  p->add_attribute(index_key_, -1);
                });
protected:
  static IntKey type_key_;
  static IntKey index_key_;

public:

  //! The supported residue types
  static ResidueType UNK,
    GLY, ALA, VAL, LEU, ILE,
    SER, THR, CYS, MET, PRO,
    ASP, ASN, GLU, GLN, LYS,
    ARG, HIS, PHE, TYR, TRP,
    ACE, NH2, ADE, URA, CYT, GUA, THY;

  ResidueType get_type() const {
    return ResidueType(get_particle()->get_value(type_key_));
  }

  void set_type(ResidueType t) {
    return get_particle()->set_value(type_key_, t.get_index());
  }

  //! The residues index in the chain
  IMP_DECORATOR_GET_SET(index, index_key_,
                        Int, unsigned int);
};

IMP_OUTPUT_OPERATOR(ResidueDecorator);

} // namespace IMP

#endif  /* __IMP_RESIDUE_DECORATOR_H */
