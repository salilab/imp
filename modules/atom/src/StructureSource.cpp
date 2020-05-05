/**
 *  \file atom/StructureSource.cpp
 *  \brief Add a Source Structure ID and Source Chain ID
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/StructureSource.h>

IMPATOM_BEGIN_NAMESPACE

StringKey StructureSource::get_source_id_key() {
  static StringKey mykey("structure source ID");
  return mykey;
}

StringKey StructureSource::get_source_chain_id_key() {
  static StringKey mykey("structure source chain ID");
  return mykey;
}

void StructureSource::show(std::ostream &out) const {
  out << get_source_id()<<" "<<get_source_chain_id();
}

IMPATOM_END_NAMESPACE
