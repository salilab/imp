/**
 *  \file IMP/atom/Residue.h     \brief A decorator for Residues.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_RESIDUE_H
#define IMPATOM_RESIDUE_H

#include <IMP/atom/atom_config.h>
#include "atom_macros.h"
#include "Hierarchy.h"
#include "Chain.h"

#include <IMP/base_types.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/Decorator.h>

IMPATOM_BEGIN_NAMESPACE

/* each static must be on a separate line because of MSVC bug C2487:
   see http://support.microsoft.com/kb/127900/
*/

IMP_DECLARE_KEY_TYPE(ResidueType, IMP_RESIDUE_TYPE_INDEX);
/** \class IMP::atom::ResidueType
    \brief The type for a residue.

    A given residue is either a Residue::LIGAND, Residue::AMINOACID,
    or Residue::NUCLEICACID.

    The standard residue types are provided with names like
    IMP::atom::GLY. New types can be added simply by creating an
    instance of ResidueType("my_residue_name"). All user-added
    residues are assumed to be ligands.

    \see Residue
*/

/** Unknown residue */
IMPATOMEXPORT extern const ResidueType UNK;
/** \relatesalso ResidueType
    glycein G*/
IMPATOMEXPORT extern const ResidueType GLY;
#ifndef IMP_DOXYGEN
/* Code currently assumes that all indices between GLY.get_index()
   and TRP.get_index() being amino acids */
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

// All further residues (including user-added residues) are ligands

/** water molecule */
IMPATOMEXPORT extern const ResidueType HOH;
/** heme */
IMPATOMEXPORT extern const ResidueType HEME;
#endif
/*@}*/


//! A decorator for a residue.
/**
   As with the Atom, the names of residues may be expanded
   dynamically. This can be easily done in an analogous manner when we
   need it.
   \ingroup hierarchy
   \ingroup decorators
 */
class IMPATOMEXPORT Residue: public Hierarchy
{
public:
  IMP_DECORATOR(Residue, Hierarchy);
  //! Add the required attributes to the particle and create a Residue
  static Residue setup_particle(Model *m, ParticleIndex pi,
                                ResidueType t= UNK,
                                int index=-1, int insertion_code = 32) {
    m->add_attribute(get_residue_type_key(), pi, t.get_index());
    m->add_attribute(get_index_key(), pi, index);
    m->add_attribute(get_insertion_code_key(), pi, insertion_code);
    // insertion code 32 is for space
    if (!Hierarchy::particle_is_instance(m, pi)) {
      Hierarchy::setup_particle(m, pi);
    }
    Residue ret(m, pi);
    ret.set_residue_type(t);
    return ret;
  }

  static Residue setup_particle(Particle *p, ResidueType t= UNK,
                                 int index=-1, int insertion_code = 32) {
    return setup_particle(p->get_model(),
                          p->get_index(), t, index, insertion_code);
  }

  //! Copy data from the other Residue to the particle p
  static Residue setup_particle(Particle *p, Residue o) {
    return setup_particle(p, o.get_residue_type(),
                          o.get_index(),
                          o.get_insertion_code());
  }

  static bool particle_is_instance(Particle *p) {
    return p->has_attribute(get_residue_type_key())
      && p->has_attribute(get_index_key())
      && p->has_attribute(get_insertion_code_key())
      && Hierarchy::particle_is_instance(p);
  }

  ResidueType get_residue_type() const {
    return ResidueType(get_particle()->get_value(get_residue_type_key()));
  }

  //! Update the stored ResidueType and the atom::Hierarchy::Name.
  void set_residue_type(ResidueType t);

  bool get_is_protein() const {
    return get_residue_type().get_index() < ADE.get_index();
  }

  bool get_is_dna() const {
    return get_residue_type().get_index() >= DADE.get_index()
      && get_residue_type().get_index() <= DTHY.get_index();
  }

  bool get_is_rna() const {
    return get_residue_type().get_index() >= ADE.get_index()
      && get_residue_type().get_index() < DADE.get_index();
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

  static IntKey get_residue_type_key();

  static IntKey get_insertion_code_key();
};

IMP_DECORATORS(Residue,Residues, Hierarchies);

/** \relatesalso Residue

    Return the residue from the same chain with one
    higher index, or Hierarchy().

    \note Currently, this function only works if
    the parent of rd is the chain. This should be fixed
    later. Ask if you need it.

    The return type is Hierarchy since the particle
    representing the next residue might not
    be a Residue particle.
 */
IMPATOMEXPORT Hierarchy get_next_residue(Residue rd);

/** \relatesalso Residue

    Return the residue from the same chain with one
    lower index, or Hierarchy().
    \see get_next_residue
 */
IMPATOMEXPORT Hierarchy get_previous_residue(Residue rd);

/** Get the residue type from the 1-letter amino acid
    code.
    \throw ValueException if an invalid character is passed.
*/
IMPATOMEXPORT ResidueType get_residue_type(char c);


/** Get the 1-letter amino acid code from the residue type.
*/
IMPATOMEXPORT char get_one_letter_code(ResidueType c);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_RESIDUE_H */
