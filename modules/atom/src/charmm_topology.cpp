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

  template<unsigned int D>
  class bond_has_atom {
    std::string name_;
  public:
    bond_has_atom(std::string name) : name_(name) {}
    bool operator()(const CHARMMBond<D> &bond) {
      return bond.contains_atom(name_);
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

  // Remove any bonds that refer to this atom
  bonds_.erase(std::remove_if(bonds_.begin(), bonds_.end(),
                              bond_has_atom<2>(name)), bonds_.end());
  angles_.erase(std::remove_if(angles_.begin(), angles_.end(),
                               bond_has_atom<3>(name)), angles_.end());
  dihedrals_.erase(std::remove_if(dihedrals_.begin(), dihedrals_.end(),
                                  bond_has_atom<4>(name)), dihedrals_.end());
  impropers_.erase(std::remove_if(impropers_.begin(), impropers_.end(),
                                  bond_has_atom<4>(name)), impropers_.end());
}

IMPATOM_END_NAMESPACE
