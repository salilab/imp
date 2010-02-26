/**
 * \file charmm_topology.cpp \brief Classes for handling CHARMM-style topology.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#include <IMP/exception.h>
#include <IMP/atom/charmm_topology.h>
#include <IMP/atom/CharmmParameters.h>
#include <IMP/atom/CHARMMAtom.h>
#include <IMP/atom/Charged.h>

#include <algorithm>

IMPATOM_BEGIN_NAMESPACE

namespace {
  class atom_has_name {
    std::string name_;
  public:
    atom_has_name(std::string name) : name_(name) {}
    bool operator()(const CHARMMAtomTopology &at) {
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

  void add_residue_bonds(const CHARMMResidueTopology *current_residue,
                         const CHARMMResidueTopology *previous_residue,
                         const CHARMMResidueTopology *next_residue,
                         const std::map<const CHARMMResidueTopology *,
                                        Hierarchy> &resmap,
                         const CharmmParameters *ff)
  {
    for (unsigned int nbond = 0; nbond < current_residue->get_number_of_bonds();
         ++nbond) {
      Atoms as = current_residue->get_bond(nbond).get_atoms(
                                   current_residue, previous_residue,
                                   next_residue, resmap);
      if (as.size() > 0) {
        Bonded b[2];
        for (unsigned int i = 0; i < 2; ++i) {
          if (Bonded::particle_is_instance(as[i])) {
            b[i] = Bonded::decorate_particle(as[i]);
          } else {
            b[i] = Bonded::setup_particle(as[i]);
          }
        }
        IMP::atom::Bond bd = bond(b[0], b[1], IMP::atom::Bond::SINGLE);

        const CHARMMBondParameters *p =
              ff->get_bond_parameters(CHARMMAtom(as[0]).get_charmm_type(),
                                      CHARMMAtom(as[1]).get_charmm_type());
        if (p) {
          bd.set_length(p->mean);
          bd.set_stiffness(p->force_constant);
        }
      }
    }
  }
}

void CHARMMResidueTopologyBase::add_atom(const CHARMMAtomTopology &atom)
{
  atoms_.push_back(atom);
}

CHARMMAtomTopology & CHARMMResidueTopologyBase::get_atom(std::string name)
{
  // A map would be more elegant here (avoid linear lookup time) but
  // a) atoms need to be ordered and b) residues rarely have more than ~30 atoms
  std::vector<CHARMMAtomTopology>::iterator it
         = std::find_if(atoms_.begin(), atoms_.end(), atom_has_name(name));
  if (it != atoms_.end()) {
    return *it;
  } else {
    IMP_THROW("atom " << name << " not found in residue topology",
              ValueException);
  }
}

const CHARMMAtomTopology & CHARMMResidueTopologyBase::get_atom(
                                                      std::string name) const
{
  std::vector<CHARMMAtomTopology>::const_iterator it
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
  std::vector<CHARMMAtomTopology>::iterator it
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

void CHARMMPatch::apply(CHARMMResidueTopology &res) const
{
  if (res.get_patched()) {
    IMP_THROW("Cannot patch an already-patched residue", ValueException);
  }

  // Copy or update atoms
  for (std::vector<CHARMMAtomTopology>::const_iterator it = atoms_.begin();
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

  // Add angles/bonds/dihedrals/impropers
  for (unsigned int i = 0; i < get_number_of_bonds(); ++i) {
    res.add_bond(get_bond(i));
  }
  for (unsigned int i = 0; i < get_number_of_angles(); ++i) {
    res.add_angle(get_angle(i));
  }
  for (unsigned int i = 0; i < get_number_of_dihedrals(); ++i) {
    res.add_dihedral(get_dihedral(i));
  }
  for (unsigned int i = 0; i < get_number_of_impropers(); ++i) {
    res.add_improper(get_improper(i));
  }

  res.set_patched(true);
}

void CHARMMResidueTopology::do_show(std::ostream &out) const
{
}

void CHARMMSegmentTopology::do_show(std::ostream &out) const
{
}

void CHARMMSegmentTopology::apply_default_patches(const CharmmParameters *ff)
{
  if (get_number_of_residues() == 0) return;

  CHARMMResidueTopology *first = get_residue(0);
  CHARMMResidueTopology *last = get_residue(get_number_of_residues() - 1);

  if (first->get_default_first_patch() != "") {
    ff->get_patch(first->get_default_first_patch()).apply(*first);
  }

  if (last->get_default_first_patch() != "") {
    // If chain contains only a single residue, allow both the first and last
    // patch to be applied to it
    if (get_number_of_residues() == 1
        && first->get_default_first_patch() != "") {
      first->set_patched(false);
    }

    ff->get_patch(last->get_default_last_patch()).apply(*last);
  }
}

void CHARMMTopology::do_show(std::ostream &out) const
{
}

void CHARMMTopology::map_residue_topology_to_hierarchy(Hierarchy hierarchy,
                                                       ResMap &resmap) const
{
  HierarchiesTemp chains = get_by_type(hierarchy, CHAIN_TYPE);
  IMP_USAGE_CHECK(chains.size() == get_number_of_segments(),
                  "Hierarchy does not match topology");

  unsigned int nseg = 0;
  for (HierarchiesTemp::iterator chainit = chains.begin();
       chainit != chains.end(); ++chainit, ++nseg) {
    CHARMMSegmentTopology *topseg = get_segment(nseg);
    HierarchiesTemp residues = get_by_type(*chainit, RESIDUE_TYPE);
    IMP_USAGE_CHECK(residues.size() == topseg->get_number_of_residues(),
                    "Hierarchy does not match topology");
    unsigned int nres = 0;
    for (HierarchiesTemp::iterator resit = residues.begin();
         resit != residues.end(); ++resit, ++nres) {
      resmap[topseg->get_residue(nres)] = *resit;
    }
  }
}

void CHARMMTopology::add_atom_types(Hierarchy hierarchy) const
{
  ResMap resmap;
  map_residue_topology_to_hierarchy(hierarchy, resmap);

  for (ResMap::iterator it = resmap.begin(); it != resmap.end(); ++it) {
    HierarchiesTemp atoms = get_by_type(it->second, ATOM_TYPE);
    for (HierarchiesTemp::iterator atit = atoms.begin(); atit != atoms.end();
         ++atit) {
      AtomType typ = Atom(*atit).get_atom_type();
      try {
        CHARMMAtom::setup_particle(*atit,
                                   it->first->get_atom(typ).get_charmm_type());
      } catch (ValueException &e) {
        IMP_WARN_ONCE("Could not determine CHARMM atom type for atom "
                      << typ << " in residue " << Residue(it->second),
                      warn_context_);
      }
    }
  }
  warn_context_.dump_warnings();
}

void CHARMMTopology::add_charges(Hierarchy hierarchy) const
{
  ResMap resmap;
  map_residue_topology_to_hierarchy(hierarchy, resmap);

  for (ResMap::iterator it = resmap.begin(); it != resmap.end(); ++it) {
    HierarchiesTemp atoms = get_by_type(it->second, ATOM_TYPE);
    for (HierarchiesTemp::iterator atit = atoms.begin(); atit != atoms.end();
         ++atit) {
      AtomType typ = Atom(*atit).get_atom_type();
      try {
        Charged::setup_particle(*atit,
                                it->first->get_atom(typ).get_charge());
      } catch (ValueException &e) {
        IMP_WARN_ONCE("Could not determine charge for atom "
                      << typ << " in residue " << Residue(it->second),
                      warn_context_);
      }
    }
  }
  warn_context_.dump_warnings();
}

void CHARMMTopology::add_bonds(Hierarchy hierarchy,
                               const CharmmParameters *ff) const
{
  ResMap resmap;
  map_residue_topology_to_hierarchy(hierarchy, resmap);

  for (CHARMMSegmentTopologyConstIterator segit = segments_begin();
       segit != segments_end(); ++segit) {
    const CHARMMSegmentTopology *seg = *segit;
    const CHARMMResidueTopology *prev = NULL;
    for (unsigned int nres = 0; nres < seg->get_number_of_residues();
         ++nres) {
      const CHARMMResidueTopology *cur = seg->get_residue(nres);
      const CHARMMResidueTopology *next =
               nres < seg->get_number_of_residues() - 1 ?
               seg->get_residue(nres + 1) : NULL;
      add_residue_bonds(cur, prev, next, resmap, ff);
      prev = cur;
    }
  }
}

IMP_LIST_IMPL(CHARMMSegmentTopology, CHARMMResidueTopology, residue,
              CHARMMResidueTopology *, CHARMMResidueTopologys, {}, {}, {});

IMP_LIST_IMPL(CHARMMTopology, CHARMMSegmentTopology, segment,
              CHARMMSegmentTopology *, CHARMMSegmentTopologys, {}, {}, {});

IMPATOM_END_NAMESPACE
