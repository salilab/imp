/**
 *  \file Chain.cpp
 *  \brief Store info for a protein chain.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/Chain.h"

IMPATOM_BEGIN_NAMESPACE

#define CTYPE_DEF(STR) const ChainType STR(ChainType::add_key(#STR))

// Polymer type definitions from http://mmcif.wwpdb.org/dictionaries/mmcif_pdbx_v50.dic/Items/_entity_poly.type.html
CTYPE_DEF(UnknownChainType);
CTYPE_DEF(DPolypeptide);
CTYPE_DEF(LPolypeptide);
CTYPE_DEF(Polydeoxyribonucleotide);
CTYPE_DEF(Polyribonucleotide);
CTYPE_DEF(DPolysaccharide);
CTYPE_DEF(LPolysaccharide);
// Shorthand formats
const ChainType Protein(LPolypeptide);
const ChainType DNA(Polydeoxyribonucleotide);
const ChainType RNA(Polyribonucleotide);

SparseStringKey Chain::get_id_key() {
  static SparseStringKey k("chain");
  return k;
}

SparseStringKey Chain::get_sequence_key() {
  static SparseStringKey k("sequence");
  return k;
}

SparseIntKey Chain::get_sequence_offset_key() {
  static SparseIntKey k("sequence offset");
  return k;
}

SparseStringKey Chain::get_uniprot_accession_key() {
  static SparseStringKey k("uniprot accession");
  return k;
}

SparseIntKey Chain::get_chain_type_key() {
  static SparseIntKey k("chain_type");
  return k;
}

void Chain::show(std::ostream &out) const { out << "Chain " << get_id(); }

Chain get_chain(Hierarchy h) {
  do {
    if (h == Hierarchy()) {
      return Chain();
    }

    if (Chain::get_is_setup(h)) {
      return Chain(h);
    }
  } while ((h = h.get_parent()));
  return Chain();
}

IMPATOM_END_NAMESPACE
