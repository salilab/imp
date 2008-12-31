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


//! A decorator for a residue.
/**
   As with the AtomDecorator, the types of residues may been to be expanded
   dynamically. This can be easily done in an analogous manner when we
   need it.
   \ingroup hierarchy
   \ingroup decorators
 */
class IMPCOREEXPORT ResidueDecorator: public DecoratorBase
{
  static IntKey type_key_;
  static IntKey index_key_;

  IMP_DECORATOR(ResidueDecorator, DecoratorBase,
                return p->has_attribute(type_key_)
                       && p->has_attribute(index_key_),
                { p->add_attribute(type_key_, -1);
                  p->add_attribute(index_key_, -1);
                });

public:

  //! The supported residue types
  /* \note each static must be on a separate line because of MSVC bug C2487:
            see http://support.microsoft.com/kb/127900/
   */
  /** @name Residue Types

   The standard residue types are provided. New types can be added simply by
   creating an instance of ResidueType("my_residue_name"). Note that methods
   such as get_is_amino_acid() will not work with user-added types.
   */
  /*@{*/

  /** Unknown residue */
  static ResidueType UNK;
  /** glycein G*/
  static ResidueType GLY;
  /** alanine A*/
  static ResidueType ALA;
  /** valine V*/
  static ResidueType VAL;
  /** leucine L*/
  static ResidueType LEU;
  /** isoleucine I*/
  static ResidueType ILE;
  /** serine S*/
  static ResidueType SER;
  /** threonine T*/
  static ResidueType THR;
  /** cystein C*/
  static ResidueType CYS;
  /** metthionine M*/
  static ResidueType MET;
  /** proline P*/
  static ResidueType PRO;
  /** aspartic acid D*/
  static ResidueType ASP;
  /** asparagine N*/
  static ResidueType ASN;
  /** glutamine Q*/
  static ResidueType GLU;
  /** glutamic acid E*/
  static ResidueType GLN;
  /** lysine K*/
  static ResidueType LYS;
  /** arginine N*/
  static ResidueType ARG;
  /** histidine H*/
  static ResidueType HIS;
  /** phynylaline F*/
  static ResidueType PHE;
  /** tyrosine Y */
  static ResidueType TYR;
  /** tryptophan W */
  static ResidueType TRP;
  /** ACE */
  static ResidueType ACE;
  /** end group */
  static ResidueType NH2;
  /* Code currently depends on all indices above ADE.get_index()
     being nucleic acid */
  /** adenine */
  static ResidueType ADE;
  /** uracil */
  static ResidueType URA;
  /** cytosine */
  static ResidueType CYT;
  /** guanine */
  static ResidueType GUA;
  /** thymine */
  static ResidueType THY;
  /*@}*/

  /** Return the ResidueType stored in the Particle */
  ResidueType get_type() const {
    return ResidueType(get_particle()->get_value(type_key_));
  }

  /** set the ResidueType stored in the Particle */
  void set_type(ResidueType t) {
    return get_particle()->set_value(type_key_, t.get_index());
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
  IMP_DECORATOR_GET_SET(index, index_key_,
                        Int, unsigned int);

  //! Get the key storing the index
  static IntKey get_index_key() {
    decorator_initialize_static_data();
    return index_key_;
  }

  //! Get the key storing the type
  static IntKey get_type_key() {
    decorator_initialize_static_data();
    return type_key_;
  }
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
