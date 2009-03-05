/**
 *  \file ResidueDecorator.h     \brief A decorator for Residues.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPATOM_RESIDUE_DECORATOR_H
#define IMPATOM_RESIDUE_DECORATOR_H

#include "config.h"
#include "macros.h"
#include <IMP/core/internal/utility.h>

#include <IMP/base_types.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/Decorator.h>

IMPATOM_BEGIN_NAMESPACE

IMP_DECLARE_KEY_TYPE(ResidueType, IMP_RESIDUE_TYPE_INDEX);


/* each static must be on a separate line because of MSVC bug C2487:
   see http://support.microsoft.com/kb/127900/
*/
/** @name Residue Types

    The standard residue types are provided. New types can be added simply by
    creating an instance of ResidueType("my_residue_name"). Note that methods
    such as get_is_amino_acid() will not work with user-added types.
    \see ResidueDecorator
*/
/*@{*/
/** Unknown residue */
IMPATOMEXPORT extern const ResidueType UNK;
/** glycein G*/
IMPATOMEXPORT extern const ResidueType GLY;
/** alanine A*/
IMPATOMEXPORT extern const ResidueType ALA;
/** valine V*/
IMPATOMEXPORT extern const ResidueType VAL;
/** leucine L*/
IMPATOMEXPORT extern const ResidueType LEU;
/** isoleucine I*/
IMPATOMEXPORT extern const ResidueType ILE;
/** serine S*/
IMPATOMEXPORT extern const ResidueType SER;
/** threonine T*/
IMPATOMEXPORT extern const ResidueType THR;
/** cystein C*/
IMPATOMEXPORT extern const ResidueType CYS;
/** metthionine M*/
IMPATOMEXPORT extern const ResidueType MET;
/** proline P*/
IMPATOMEXPORT extern const ResidueType PRO;
/** aspartic acid D*/
IMPATOMEXPORT extern const ResidueType ASP;
/** asparagine N*/
IMPATOMEXPORT extern const ResidueType ASN;
/** glutamine Q*/
IMPATOMEXPORT extern const ResidueType GLU;
/** glutamic acid E*/
IMPATOMEXPORT extern const ResidueType GLN;
/** lysine K*/
IMPATOMEXPORT extern const ResidueType LYS;
/** arginine N*/
IMPATOMEXPORT extern const ResidueType ARG;
/** histidine H*/
IMPATOMEXPORT extern const ResidueType HIS;
/** phynylaline F*/
IMPATOMEXPORT extern const ResidueType PHE;
/** tyrosine Y */
IMPATOMEXPORT extern const ResidueType TYR;
/** tryptophan W */
IMPATOMEXPORT extern const ResidueType TRP;
/** ACE */
IMPATOMEXPORT extern const ResidueType ACE;
/** end group */
IMPATOMEXPORT extern const ResidueType NH2;
/* Code currently depends on all indices above ADE.get_index()
   being nucleic acid */
/** adenine */
IMPATOMEXPORT extern const ResidueType ADE;
/** uracil */
IMPATOMEXPORT extern const ResidueType URA;
/** cytosine */
IMPATOMEXPORT extern const ResidueType CYT;
/** guanine */
IMPATOMEXPORT extern const ResidueType GUA;
/** thymine */
IMPATOMEXPORT extern const ResidueType THY;
/*@}*/


//! A decorator for a residue.
/**
   As with the AtomDecorator, the types of residues may be expanded
   dynamically. This can be easily done in an analogous manner when we
   need it.
   \ingroup hierarchy
   \ingroup decorators
 */
class IMPATOMEXPORT ResidueDecorator: public Decorator
{
public:
  IMP_DECORATOR(ResidueDecorator, Decorator)
  //! create a decorator from the provided information
  static ResidueDecorator create(Particle *p, ResidueType t= UNK,
                                 int index=-1, int insertion_code = 32) {
    p->add_attribute(get_type_key(), t.get_index());
    p->add_attribute(get_index_key(), index);
    p->add_attribute(get_insertion_code_key(), insertion_code);
    // insertion code 32 is for space
    return ResidueDecorator(p);
  }

  //! Copy data from the o
  static ResidueDecorator create(Particle *p, ResidueDecorator o) {
    p->add_attribute(get_type_key(), o.get_type().get_index());
    p->add_attribute(get_index_key(), o.get_index());
    p->add_attribute(get_insertion_code_key(), o.get_insertion_code());
    return ResidueDecorator(p);
  }

  static bool is_instance_of(Particle *p) {
    return p->has_attribute(get_type_key())
    && p->has_attribute(get_index_key())
    && p->has_attribute(get_insertion_code_key());
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
  char get_insertion_code() const {
    return char(get_particle()->get_value(get_insertion_code_key()));
  }

  //! set the insertion code
  void set_insertion_code(char insertion_code) {
    return get_particle()->set_value(get_insertion_code_key(), insertion_code);
  }

  //! Get the key storing the index
  static IntKey get_index_key();

  //! Get the key storing the type
  static IntKey get_type_key();

  //! Get the key storing the index
  static IntKey get_insertion_code_key();
};

IMP_OUTPUT_OPERATOR(ResidueDecorator);

//! Return the residue type from the three letter code in the PDB
/** The string should be capitalized, as in the PDB.
    \throw ValueException if nm is invalid.
    \relates ResidueDecorator
    \relates ResidueType
 */
IMPATOMEXPORT ResidueType residue_type_from_pdb_string(std::string nm);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_RESIDUE_DECORATOR_H */
