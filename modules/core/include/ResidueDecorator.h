/**
 *  \file ResidueDecorator.h     \brief A decorator for Residues.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_RESIDUE_DECORATOR_H
#define IMPCORE_RESIDUE_DECORATOR_H

#include "config.h"
#include "internal/utility.h"

#include <IMP/base_types.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/DecoratorBase.h>

IMPCORE_BEGIN_NAMESPACE

IMP_DECLARE_KEY_TYPE(ResidueType, IMP_RESIDUE_TYPE_INDEX);


/* each static must be on a separate line because of MSVC bug C2487:
   see http://support.microsoft.com/kb/127900/
*/
/** @name Residue Types

    The standard residue types are provided. New types can be added simply by
    creating an instance of ResidueType("my_residue_name"). Note that methods
    such as get_is_amino_acid() will not work with user-added types.
    \relates ResidueDecorator
*/
/*@{*/
/** Unknown residue */
IMPCOREEXPORT extern const ResidueType UNK;
/** glycein G*/
IMPCOREEXPORT extern const ResidueType GLY;
/** alanine A*/
IMPCOREEXPORT extern const ResidueType ALA;
/** valine V*/
IMPCOREEXPORT extern const ResidueType VAL;
/** leucine L*/
IMPCOREEXPORT extern const ResidueType LEU;
/** isoleucine I*/
IMPCOREEXPORT extern const ResidueType ILE;
/** serine S*/
IMPCOREEXPORT extern const ResidueType SER;
/** threonine T*/
IMPCOREEXPORT extern const ResidueType THR;
/** cystein C*/
IMPCOREEXPORT extern const ResidueType CYS;
/** metthionine M*/
IMPCOREEXPORT extern const ResidueType MET;
/** proline P*/
IMPCOREEXPORT extern const ResidueType PRO;
/** aspartic acid D*/
IMPCOREEXPORT extern const ResidueType ASP;
/** asparagine N*/
IMPCOREEXPORT extern const ResidueType ASN;
/** glutamine Q*/
IMPCOREEXPORT extern const ResidueType GLU;
/** glutamic acid E*/
IMPCOREEXPORT extern const ResidueType GLN;
/** lysine K*/
IMPCOREEXPORT extern const ResidueType LYS;
/** arginine N*/
IMPCOREEXPORT extern const ResidueType ARG;
/** histidine H*/
IMPCOREEXPORT extern const ResidueType HIS;
/** phynylaline F*/
IMPCOREEXPORT extern const ResidueType PHE;
/** tyrosine Y */
IMPCOREEXPORT extern const ResidueType TYR;
/** tryptophan W */
IMPCOREEXPORT extern const ResidueType TRP;
/** ACE */
IMPCOREEXPORT extern const ResidueType ACE;
/** end group */
IMPCOREEXPORT extern const ResidueType NH2;
/* Code currently depends on all indices above ADE.get_index()
   being nucleic acid */
/** adenine */
IMPCOREEXPORT extern const ResidueType ADE;
/** uracil */
IMPCOREEXPORT extern const ResidueType URA;
/** cytosine */
IMPCOREEXPORT extern const ResidueType CYT;
/** guanine */
IMPCOREEXPORT extern const ResidueType GUA;
/** thymine */
IMPCOREEXPORT extern const ResidueType THY;
/*@}*/


//! A decorator for a residue.
/**
   As with the AtomDecorator, the types of residues may be expanded
   dynamically. This can be easily done in an analogous manner when we
   need it.
   \ingroup hierarchy
   \ingroup decorators
 */
class IMPCOREEXPORT ResidueDecorator: public DecoratorBase
{
public:
  IMP_DECORATOR(ResidueDecorator, DecoratorBase)

  static ResidueDecorator create(Particle *p, ResidueType t= UNK,
                                 int index=-1, int icode = 32) {
    p->add_attribute(get_type_key(), t.get_index());
    p->add_attribute(get_index_key(), index);
    p->add_attribute(get_icode_key(), icode); // 32 is for space
    return ResidueDecorator(p);
  }
  //! Return true if the particle is a ResidueDecorator
  static bool is_instance_of(Particle *p) {
    return p->has_attribute(get_type_key())
    && p->has_attribute(get_index_key());
  }

  /** Return the ResidueType stored in the Particle */
  ResidueType get_type() const {
    return ResidueType(get_particle()->get_value(get_type_key()));
  }

  /** set the ResidueType stored in the Particle */
  void set_type(ResidueType t) {
    return get_particle()->set_value(get_type_key(), t.get_index());
  }

  /** Return true if the residue is an amino acid */
  bool get_is_amino_acid() const {
    return !get_is_nucleic_acid();
  }

  /** Return true if the residue is a nucleic acid */
  bool get_is_nucleic_acid() const {
    return get_type().get_index() >= ADE.get_index();
  }
  //! The residues index in the chain
  IMP_DECORATOR_GET_SET(index, get_index_key(),
                        Int, Int);

  //! Return the insertion code of the residue
  char get_icode() const {
    return char(get_particle()->get_value(get_icode_key()));
  }

  //! set the insertion code
  void set_icode(char icode) {
    return get_particle()->set_value(get_icode_key(), icode);
  }

  //! Get the key storing the index
  static IntKey get_index_key();

  //! Get the key storing the type
  static IntKey get_type_key();

  //! Get the key storing the index
  static IntKey get_icode_key();
};

IMP_OUTPUT_OPERATOR(ResidueDecorator);

//! Return the residue type from the three letter code in the PDB
/** The string should be capitalized, as in the PDB.
    \throw ValueException if nm is invalid.
    \relates ResidueDecorator
    \relates ResidueType
 */
IMPCOREEXPORT ResidueType residue_type_from_pdb_string(std::string nm);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_RESIDUE_DECORATOR_H */
