/**
 *  \file IMP/atom/Chain.h
 *  \brief Store the chain ID
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_CHAIN_H
#define IMPATOM_CHAIN_H

#include <IMP/atom/atom_config.h>
#include "Hierarchy.h"
#include <IMP/macros.h>
#include <IMP/Decorator.h>
#include <IMP/base_types.h>

IMPATOM_BEGIN_NAMESPACE

typedef Key<IMP_CHAIN_TYPE_INDEX> ChainType;
IMP_VALUES(ChainType, ChainTypes);

/** \class IMP::atom::ChainType
    \brief The type for a chain.

    A given chain can be a Protein, DNA, or RNA polymer (or a few other much
    more rare types).

    The standard chain types are provided with names like IMP::atom::Protein.

    \see Chain
*/

// To add a new chain type, add both here and in src/Chain.cpp.
// RMF may also need to be updated to understand the new type.

//! Unknown chain type
IMPATOMEXPORT extern const ChainType UnknownChainType;
//! Polypeptide(D)
IMPATOMEXPORT extern const ChainType DPolypeptide;
//! Polypeptide(L)
IMPATOMEXPORT extern const ChainType LPolypeptide;
//! DNA
IMPATOMEXPORT extern const ChainType Polydeoxyribonucleotide;
//! RNA
IMPATOMEXPORT extern const ChainType Polyribonucleotide;
//! Polysaccharide(D)
IMPATOMEXPORT extern const ChainType DPolysaccharide;
//! Polysaccharide(L)
IMPATOMEXPORT extern const ChainType LPolysaccharide;
//! Shorthand for IMP::atom::LPolypeptide
IMPATOMEXPORT extern const ChainType Protein;
//! Shorthand for IMP::atom::Polydeoxyribonucleotide
IMPATOMEXPORT extern const ChainType DNA;
//! Shorthand for IMP::atom::Polyribonucleotide
IMPATOMEXPORT extern const ChainType RNA;


//! Store info for a chain of a protein
/** \see Hierarchy
 */
class IMPATOMEXPORT Chain : public Hierarchy {

  static void do_setup_particle(Model *m, ParticleIndex pi,
                                std::string id) {
    m->add_attribute(get_id_key(), pi, id);
    m->add_attribute(get_sequence_key(), pi, "");
    m->add_attribute(get_sequence_offset_key(), pi, 0);
    m->add_attribute(get_uniprot_accession_key(), pi, "");
    m->add_attribute(get_label_asym_id_key(), pi, "");
    m->add_attribute(get_chain_type_key(), pi, UnknownChainType.get_index());
    if (!Hierarchy::get_is_setup(m, pi)) {
      Hierarchy::setup_particle(m, pi);
    }
  }
  static void do_setup_particle(Model *m, ParticleIndex pi, char c) {
    do_setup_particle(m, pi, std::string(1, c));
  }
  static void do_setup_particle(Model *m, ParticleIndex pi, Chain o) {
    do_setup_particle(m, pi, o.get_id());
  }

 public:
  IMP_DECORATOR_METHODS(Chain, Hierarchy);
  IMP_DECORATOR_SETUP_1(Chain, std::string, id);
  IMP_DECORATOR_SETUP_1(Chain, char, id);
  IMP_DECORATOR_SETUP_1(Chain, Chain, other);

  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_id_key(), pi) &&
           m->get_has_attribute(get_sequence_key(), pi) &&
           m->get_has_attribute(get_sequence_offset_key(), pi) &&
           m->get_has_attribute(get_uniprot_accession_key(), pi) &&
           m->get_has_attribute(get_chain_type_key(), pi) &&
           m->get_has_attribute(get_label_asym_id_key(), pi) &&
           Hierarchy::get_is_setup(m, pi);
  }

  //! Return the (author-provided) chain id
  std::string get_id() const {
    return get_model()->get_attribute(get_id_key(), get_particle_index());
  }

  //! Set the (author-provided) chain id
  void set_id(std::string c) {
    get_model()->set_attribute(get_id_key(), get_particle_index(), c);
  }

  //! Return the mmCIF asym ID
  std::string get_label_asym_id() const {
    return get_model()->get_attribute(get_label_asym_id_key(),
                                      get_particle_index());
  }

  //! Set the mmCIF asym ID
  void set_label_asym_id(std::string c) {
    get_model()->set_attribute(get_label_asym_id_key(),
                               get_particle_index(), c);
  }

  //! Return the primary sequence (or any empty string)
  std::string get_sequence() const {
    return get_model()->get_attribute(get_sequence_key(), get_particle_index());
  }

  //! Set the primary sequence, as a string
  /** Usually the primary sequence of a chain can be uniquely deduced by
      iterating over all child Residue decorators and querying their type.
      However, this may not be possible in all cases (e.g. if there are gaps
      in the sequence or parts that are not explicitly represented).

      \note The sequence set here should be consistent with that of any
            children of this Chain. This is not currently enforced.
      */
  void set_sequence(std::string sequence) {
    get_model()->set_attribute(get_sequence_key(), get_particle_index(),
                               sequence);
  }

  //! Return the offset from the sequence numbering to residue indexes
  int get_sequence_offset() const {
    return get_model()->get_attribute(get_sequence_offset_key(),
                                      get_particle_index());
  }

  //! Set the offset from the sequence numbering to residue indexes
  /** This offset is added to 1-based indexes into the chain primary sequence
      (see set_sequence()) to get corresponding residue indexes. By default,
      the offset is zero, corresponding to residues being numbered from 1.
      For example, if residues were instead numbered starting from 0, the
      offset would be -1.
    */
  void set_sequence_offset(int offset) {
    get_model()->set_attribute(get_sequence_offset_key(), get_particle_index(),
                               offset);
  }

  //! Return the UniProt accession for the chain's sequence
  std::string get_uniprot_accession() const {
    return get_model()->get_attribute(get_uniprot_accession_key(),
                                      get_particle_index());
  }

  //! Set the UniProt accession for the chain's sequence
  void set_uniprot_accession(std::string accession) {
    get_model()->set_attribute(get_uniprot_accession_key(),
                               get_particle_index(), accession);
  }

  //! Return the chain type
  ChainType get_chain_type() const {
    return ChainType(get_model()->get_attribute(get_chain_type_key(),
                                                  get_particle_index()));
  }


  //! Set the chain type, using IMP::atom::ChainType
  /** Usually the chain type can be uniquely deduced by
      iterating over all child Residue decorators and querying their type.
      However, this may not be possible in all cases.

      \note The type set here should be consistent with that of any
            children of this Chain. This is not currently enforced.
            One can use IMP::atom::Residue::get_is_protein() on the leaves.
      */
  void set_chain_type(ChainType t) {
    get_model()->set_attribute(get_chain_type_key(), get_particle_index(),
                               t.get_index());
  }


  //! The key used to store the author-provided chain ID
  static SparseStringKey get_id_key();

  //! The key used to store the mmCIF asym ID
  static SparseStringKey get_label_asym_id_key();

  //! The key used to store the primary sequence
  static SparseStringKey get_sequence_key();

  //! The key used to store the sequence offset
  static SparseIntKey get_sequence_offset_key();

  //! The key used to store the UniProt accession
  static SparseStringKey get_uniprot_accession_key();

  //! The key used to store the polymer type
  static SparseIntKey get_chain_type_key();
};

IMP_DECORATORS(Chain, Chains, Hierarchies);

//! Get the containing chain or Chain() if there is none
IMPATOMEXPORT Chain get_chain(Hierarchy h);

//! Walk up the hierarchy to determine the chain id.
IMPATOMEXPORT std::string get_chain_id(Hierarchy h);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_CHAIN_H */
