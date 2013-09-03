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
#include <IMP/kernel/Particle.h>
#include <IMP/kernel/Model.h>
#include <IMP/Decorator.h>

IMPATOM_BEGIN_NAMESPACE

/* each static must be on a separate line because of MSVC bug C2487:
   see http://support.microsoft.com/kb/127900/
*/
typedef Key<IMP_RESIDUE_TYPE_INDEX, true> ResidueType;
IMP_VALUES(ResidueType, ResidueTypes);

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
/** See ResidueType
    glycine G*/
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
class IMPATOMEXPORT Residue : public Hierarchy {
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                ResidueType t = UNK,
                                int index = -1,
                                int insertion_code = 32) {
    m->add_attribute(get_residue_type_key(), pi, t.get_index());
    m->add_attribute(get_index_key(), pi, index);
    m->add_attribute(get_insertion_code_key(), pi, insertion_code);
    // insertion code 32 is for space
    if (!Hierarchy::get_is_setup(m, pi)) {
      Hierarchy::setup_particle(m, pi);
    }
    Residue ret(m, pi);
    ret.set_residue_type(t);
  }
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                const Residue &o) {
    do_setup_particle(m, pi, o.get_residue_type(),
                      o.get_index(),
                      o.get_insertion_code());
  }
 public:
  IMP_DECORATOR_METHODS(Residue, Hierarchy);
  IMP_DECORATOR_SETUP_3(Residue, ResidueType, t,
                        int, index, int, insertion_code);
  /** Setup the particle as a Residue with the passed type and index. */
  IMP_DECORATOR_SETUP_2(Residue, ResidueType, t,
                        int, index);
  IMP_DECORATOR_SETUP_1(Residue, ResidueType, t);
  IMP_DECORATOR_SETUP_1(Residue, Residue, other);

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return m->get_has_attribute(get_residue_type_key(), pi) &&
           m->get_has_attribute(get_index_key(), pi) &&
           m->get_has_attribute(get_insertion_code_key(), pi) &&
           Hierarchy::get_is_setup(m, pi);
  }

  ResidueType get_residue_type() const {
    return ResidueType(get_model()->get_attribute(get_residue_type_key(),
                                                  get_particle_index()));
  }

  //! Update the stored ResidueType and the atom::Hierarchy::Name.
  void set_residue_type(ResidueType t);

  bool get_is_protein() const {
    return get_residue_type().get_index() < ADE.get_index();
  }

  bool get_is_dna() const {
    return get_residue_type().get_index() >= DADE.get_index() &&
           get_residue_type().get_index() <= DTHY.get_index();
  }

  bool get_is_rna() const {
    return get_residue_type().get_index() >= ADE.get_index() &&
           get_residue_type().get_index() < DADE.get_index();
  }

  //! The residues index in the chain
  IMP_DECORATOR_GET_SET(index, get_index_key(), Int, Int);

  char get_insertion_code() const {
    return char(get_model()->get_attribute(get_insertion_code_key(),
                                           get_particle_index()));
  }

  void set_insertion_code(char insertion_code) {
    return get_model()->set_attribute(get_insertion_code_key(),
                                      get_particle_index(),
                                      insertion_code);
  }

  static IntKey get_index_key();

  static IntKey get_residue_type_key();

  static IntKey get_insertion_code_key();
};

IMP_DECORATORS(Residue, Residues, Hierarchies);

/** See Residue

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

/** See Residue

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

#endif /* IMPATOM_RESIDUE_H */
