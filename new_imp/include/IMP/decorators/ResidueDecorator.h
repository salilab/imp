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
#include <vector>
#include <deque>

namespace IMP
{

//! A decorator for a residue.
class IMPDLLEXPORT ResidueDecorator
{
  IMP_DECORATOR(ResidueDecorator,
                return p->has_attribute(type_key_)
                       && p->has_attribute(index_key_),
                { p->add_attribute(type_key_, INV);
                  p->add_attribute(index_key_, -1);
                });
protected:
  static bool keys_initialized_;
  static IntKey type_key_;
  static IntKey index_key_;


public:
  //! The supported residue types
  enum Type {
    GLY=0, ALA, VAL, LEU, ILE,
    SER, THR, CYS, MET, PRO,
    ASP, ASN, GLU, GLN, LYS,
    ARG, HIS, PHE, TYR, TRP,
    ACE, NH2, ADE, URA, CYT, GUA, THY, INV
  };

  IMP_DECORATOR_GET_SET(type, type_key_,
                        Int, Type);
  //! The residues index in the chain
  IMP_DECORATOR_GET_SET(index, index_key_,
                        Int, unsigned int);

};

IMP_OUTPUT_OPERATOR(ResidueDecorator);

} // namespace IMP

#endif  /* __IMP_RESIDUE_DECORATOR_H */
