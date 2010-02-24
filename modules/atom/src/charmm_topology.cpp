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

void CHARMMPatch::apply(CHARMMResidueTopology &res)
{
  if (res.get_patched()) {
    IMP_THROW("Cannot patch an already-patched residue", ValueException);
  }

  // Copy or update atoms
  for (std::vector<CHARMMAtom>::const_iterator it = atoms_.begin();
       it != atoms_.end(); ++it) {
    try {
      res.get_atom(it->get_name()) = *it;
    } catch (ValueException &e) {
      res.add_atom(*it);
    }
  }

  // Delete atoms
  for (std::vector<std::string>::const_iterator it = deleted_atoms_.begin();
       it != deleted_atoms_.end(); ++it) {
    try {
      res.delete_atom(*it);
    } catch (ValueException &e) {
      // ignore atoms that don't exist to start with
    }
  }

  // Todo: add bonds/angles/dihedrals; handle two-residue patches (DISU, LINK)
  res.set_patched(true);
}

IMPATOM_END_NAMESPACE
