/**
 *  \file IMP/pmi/CrossLinkRestraintSet.cpp
 *  \brief A RestraintSet subclass to track cross-link metadata.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/pmi/CrossLinkRestraintSet.h>

IMPPMI_BEGIN_NAMESPACE

RestraintInfo *CrossLinkRestraintSet::get_static_info() const {
  IMP_NEW(RestraintInfo, ri, ());
  ri->add_string("type", "IMP.pmi.CrossLinkingMassSpectrometryRestraint");
  ri->add_float("linker length", length_);
  ri->add_float("slope", slope_);
  ri->add_filename("filename", filename_);
  if (!auth_name_.empty()) {
    ri->add_string("linker author name", auth_name_);
    if (!chemical_name_.empty()) {
      ri->add_string("linker chemical name", chemical_name_);
    }
    if (!smiles_.empty()) {
      ri->add_string("linker smiles", smiles_);
    }
    if (!smiles_canonical_.empty()) {
      ri->add_string("linker smiles canonical", smiles_canonical_);
    }
    if (!inchi_.empty()) {
      ri->add_string("linker inchi", inchi_);
    }
    if (!inchi_key_.empty()) {
      ri->add_string("linker inchi key", inchi_key_);
    }
  }
  return ri.release();
}

IMP_OBJECT_SERIALIZE_IMPL(IMP::pmi::CrossLinkRestraintSet);

IMPPMI_END_NAMESPACE
