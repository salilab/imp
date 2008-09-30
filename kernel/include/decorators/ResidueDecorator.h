/**
 *  \file ResidueDecorator.h     \brief A decorator for Residues.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_RESIDUE_DECORATOR_H
#define __IMP_RESIDUE_DECORATOR_H

#include "../base_types.h"
#include "../Particle.h"
#include "../Model.h"
#include "../DecoratorBase.h"
#include "utility.h"

namespace IMP
{

IMP_DECLARE_KEY_TYPE(ResidueType, IMP_RESIDUE_TYPE_INDEX);


//! A decorator for a residue.
/**
   As with the AtomDecorator, the types of residues may been to be expanded
   dynamically. This can be easily done in an analogous manner when we
   need it.
   \ingroup hierarchy
   \ingroup decorators
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
  /** \note each static must be on a separate line because of MSVC bug C2487:
            see http://support.microsoft.com/kb/127900/
   */
  static ResidueType UNK;
  static ResidueType GLY;
  static ResidueType ALA;
  static ResidueType VAL;
  static ResidueType LEU;
  static ResidueType ILE;
  static ResidueType SER;
  static ResidueType THR;
  static ResidueType CYS;
  static ResidueType MET;
  static ResidueType PRO;
  static ResidueType ASP;
  static ResidueType ASN;
  static ResidueType GLU;
  static ResidueType GLN;
  static ResidueType LYS;
  static ResidueType ARG;
  static ResidueType HIS;
  static ResidueType PHE;
  static ResidueType TYR;
  static ResidueType TRP;
  static ResidueType ACE;
  static ResidueType NH2;
  /* Code currently depends on all indices above ADE.get_index()
     being nucleic acid */
  static ResidueType ADE;
  static ResidueType URA;
  static ResidueType CYT;
  static ResidueType GUA;
  static ResidueType THY;

  ResidueType get_type() const {
    return ResidueType(get_particle()->get_value(type_key_));
  }

  void set_type(ResidueType t) {
    return get_particle()->set_value(type_key_, t.get_index());
  }

  bool get_is_amino_acid() const {
    return !get_is_nucleic_acid();
  }

  bool get_is_nucleic_acid() const {
    return get_type().get_index() >= ADE.get_index();
  }
  //! The residues index in the chain
  IMP_DECORATOR_GET_SET(index, index_key_,
                        Int, unsigned int);
};

IMP_OUTPUT_OPERATOR(ResidueDecorator);

} // namespace IMP

#endif  /* __IMP_RESIDUE_DECORATOR_H */
