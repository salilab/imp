/**
 *  \file Chain.cpp
 *  \brief Store info for a protein chain.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
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

StringKey Chain::get_id_key() {
  static StringKey k("chain");
  return k;
}

StringKey Chain::get_sequence_key() {
  static StringKey k("sequence");
  return k;
}

IntKey Chain::get_chain_type_key() {
  static IntKey k("chain_type");
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
