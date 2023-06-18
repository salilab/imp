/**
 *  \file IMP/pmi/CrossLinkRestraintSet.h
 *  \brief A RestraintSet subclass to track cross-link metadata.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPPMI_CROSS_LINK_RESTRAINT_SET_H
#define IMPPMI_CROSS_LINK_RESTRAINT_SET_H

#include "pmi_config.h"
#include <IMP/RestraintSet.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPPMI_BEGIN_NAMESPACE

//! A RestraintSet subclass to track cross-link metadata.
/** This is provided so that we can write information about a set of
    cross-links to an RMF file (e.g. the CSV file from which they were read).
 */
class IMPPMIEXPORT CrossLinkRestraintSet : public RestraintSet
{
  std::string filename_;
  double length_, slope_;
  std::string auth_name_, chemical_name_, smiles_, smiles_canonical_, inchi_,
              inchi_key_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<RestraintSet>(this),
       filename_, length_, slope_, auth_name_, chemical_name_, smiles_,
       smiles_canonical_, inchi_, inchi_key_);
  }
  IMP_OBJECT_SERIALIZE_DECL(CrossLinkRestraintSet);

 public:
  CrossLinkRestraintSet(Model *m,
                        const std::string &name = "CrossLinkRestraintSet %1%")
          : RestraintSet(m, name) {}
  CrossLinkRestraintSet() {}

  void set_metadata(std::string filename, double length, double slope) {
    filename_ = filename;
    length_ = length;
    slope_ = slope;
  }

  void set_linker_auth_name(std::string name) { auth_name_ = name; }
  void set_linker_chemical_name(std::string name) { chemical_name_ = name; }
  void set_linker_smiles(std::string name) { smiles_ = name; }
  void set_linker_smiles_canonical(std::string name) {
    smiles_canonical_ = name;
  }
  void set_linker_inchi(std::string name) { inchi_ = name; }
  void set_linker_inchi_key(std::string name) { inchi_key_ = name; }

  virtual RestraintInfo *get_static_info() const override;

  IMP_OBJECT_METHODS(CrossLinkRestraintSet);
};

IMPPMI_END_NAMESPACE

#endif  /* IMPPMI_CROSS_LINK_RESTRAINT_SET_H */
