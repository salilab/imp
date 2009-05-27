/**
 *  \file Residue.h     \brief A decorator for Residues.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPATOM_RESIDUE_H
#define IMPATOM_RESIDUE_H

#include "config.h"
#include "macros.h"
#include "Hierarchy.h"
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

    The standard residue types are provided with names like
    IMP::atom::GLY.  New types can be added simply by creating an
    instance of ResidueType("my_residue_name"). Note that methods such
    as Residue::get_is_amino_acid() will not work with
    user-added types.

    \see Residue
*/
/*@{*/
/** Unknown residue */
IMPATOMEXPORT extern const ResidueType UNK;
#ifndef IMP_DOXYGEN
/* Code currently assumes that all indices between GLY.get_index()
   and TRP.get_index() being amino acids */
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
/* Code currently assumes that all indices between ADE.get_index()
   and DTHY.get_index() being nucleic acid */
/** adenine (RNA) */
IMPATOMEXPORT extern const ResidueType ADE;
/** uracil (RNA) */
IMPATOMEXPORT extern const ResidueType URA;
/** cytosine (RNA) */
IMPATOMEXPORT extern const ResidueType CYT;
/** guanine (RNA) */
IMPATOMEXPORT extern const ResidueType GUA;
/** thymine (RNA) */
IMPATOMEXPORT extern const ResidueType THY;
/** adenine (DNA) */
IMPATOMEXPORT extern const ResidueType DADE;
/** uracil (DNA) */
IMPATOMEXPORT extern const ResidueType DURA;
/** cytosine (DNA) */
IMPATOMEXPORT extern const ResidueType DCYT;
/** guanine (DNA) */
IMPATOMEXPORT extern const ResidueType DGUA;
/** thymine (DNA) */
IMPATOMEXPORT extern const ResidueType DTHY;
#endif
/*@}*/


//! A decorator for a residue.
/**
   As with the Atom, the types of residues may be expanded
   dynamically. This can be easily done in an analogous manner when we
   need it.
   \ingroup hierarchy
   \ingroup decorators
 */
class IMPATOMEXPORT Residue: public Hierarchy
{
public:
  IMP_DECORATOR(Residue, Hierarchy)
  //! Add the required attributes to the particle and create a Residue
  static Residue create(Particle *p, ResidueType t= UNK,
                                 int index=-1, int insertion_code = 32) {
    p->add_attribute(get_type_key(), t.get_index());
    p->add_attribute(get_index_key(), index);
    p->add_attribute(get_insertion_code_key(), insertion_code);
    // insertion code 32 is for space
    Hierarchy::create(p,
                     Hierarchy::UNKNOWN);
    Residue ret(p);
    ret.set_residue_type(t);
    return ret;
  }

  //! Copy data from the other Residue to the particle p
  static Residue create(Particle *p, Residue o) {
    p->add_attribute(get_type_key(), o.get_residue_type().get_index());
    p->add_attribute(get_index_key(), o.get_index());
    p->add_attribute(get_insertion_code_key(), o.get_insertion_code());
    Hierarchy::create(p,
              static_cast<Hierarchy>(o).get_type());
    return Residue(p);
  }

  static bool is_instance_of(Particle *p) {
    return p->has_attribute(get_type_key())
      && p->has_attribute(get_index_key())
      && p->has_attribute(get_insertion_code_key())
      && Hierarchy::is_instance_of(p);
  }

  ResidueType get_residue_type() const {
    return ResidueType(get_particle()->get_value(get_type_key()));
  }

  //! Update the stored ResidueType and the MolecularHierarchy::Type.
  void set_residue_type(ResidueType t) {
    get_particle()->set_value(get_type_key(), t.get_index());
    if (get_residue_type().get_index() >= GLY.get_index() &&
        get_residue_type().get_index() <= TRP.get_index()) {
      Hierarchy
        ::set_type(Hierarchy::RESIDUE);
    } else if (get_residue_type().get_index() >= ADE.get_index() &&
               get_residue_type().get_index() <= DTHY.get_index()) {
      Hierarchy
        ::set_type(Hierarchy::NUCLEICACID);
    } else {
      Hierarchy
        ::set_type(Hierarchy::FRAGMENT);
    }
  }

  //! The residues index in the chain
  IMP_DECORATOR_GET_SET(index, get_index_key(),
                        Int, Int);

  char get_insertion_code() const {
    return char(get_particle()->get_value(get_insertion_code_key()));
  }

  void set_insertion_code(char insertion_code) {
    return get_particle()->set_value(get_insertion_code_key(), insertion_code);
  }

  static IntKey get_index_key();

  static IntKey get_type_key();

  static IntKey get_insertion_code_key();
};

IMP_OUTPUT_OPERATOR(Residue);

//! Return the residue type from the three letter code in the PDB
/** The string should be capitalized, as in the PDB.
    \throw ValueException if nm is invalid.
    \relatesalso Residue
    \relatesalso ResidueType
 */
IMPATOMEXPORT ResidueType residue_type_from_pdb_string(std::string nm);

IMPATOMEXPORT char get_chain(Residue rd);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_RESIDUE_H */
