/**
 * \file charmm_topology.cpp \brief Classes for handling CHARMM-style topology.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#include <IMP/exception.h>
#include <IMP/atom/charmm_topology.h>

#include <algorithm>

IMPATOM_BEGIN_NAMESPACE

namespace {
  class atom_has_name {
    std::string name_;
  public:
    atom_has_name(std::string name) : name_(name) {}
    bool operator()(const CHARMMAtom &at) {
      return (at.get_name() == name_);
    }
  };
}

void CHARMMResidueTopologyBase::add_atom(const CHARMMAtom &atom)
{
  atoms_.push_back(atom);
}

CHARMMAtom & CHARMMResidueTopologyBase::get_atom(std::string name)
{
  // A map would be more elegant here (avoid linear lookup time) but
  // a) atoms need to be ordered and b) residues rarely have more than ~30 atoms
  std::vector<CHARMMAtom>::iterator it
         = std::find_if(atoms_.begin(), atoms_.end(), atom_has_name(name));
  if (it != atoms_.end()) {
    return *it;
  } else {
    IMP_THROW("atom " << name << " not found in residue topology",
              ValueException);
  }
}

void CHARMMIdealResidueTopology::delete_atom(std::string name)
{
  std::vector<CHARMMAtom>::iterator it
         = std::find_if(atoms_.begin(), atoms_.end(), atom_has_name(name));
  if (it != atoms_.end()) {
    atoms_.erase(it);
  } else {
    IMP_THROW("atom " << name << " not found in residue topology",
              ValueException);
  }
}

IMPATOM_END_NAMESPACE
