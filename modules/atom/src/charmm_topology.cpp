/**
 * \file charmm_topology.cpp \brief Classes for handling CHARMM-style topology.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP/exception.h>
#include <IMP/constants.h>
#include <IMP/atom/charmm_topology.h>
#include <IMP/atom/CHARMMParameters.h>
#include <IMP/atom/CHARMMAtom.h>
#include <IMP/atom/Charged.h>
#include <IMP/atom/angle_decorators.h>

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
                         const CHARMMParameters *ff,
                         Particles &ps)
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
        IMP::atom::Bond bd = create_bond(b[0], b[1], IMP::atom::Bond::SINGLE);

        try {
          const CHARMMBondParameters &p =
                ff->get_bond_parameters(CHARMMAtom(as[0]).get_charmm_type(),
                                        CHARMMAtom(as[1]).get_charmm_type());
          bd.set_length(p.ideal);
          // Note that CHARMM uses kx^2 rather than (1/2)kx^2 for harmonic
          // restraints, so we need to add a factor of two; stiffness is also
          // incorporated into x, so is the sqrt of the force constant
          bd.set_stiffness(std::sqrt(p.force_constant * 2.0));
        } catch (const IndexException &e) {
          // If no parameters, warn only
          IMP_WARN(e.what() << std::endl);
        }
        ps.push_back(bd);
      }
    }
  }

  void add_residue_impropers(const CHARMMResidueTopology *current_residue,
                             const CHARMMResidueTopology *previous_residue,
                             const CHARMMResidueTopology *next_residue,
                             const std::map<const CHARMMResidueTopology *,
                                            Hierarchy> &resmap,
                             const CHARMMParameters *ff,
                             Particles &ps)
  {
    for (unsigned int nimpr = 0;
         nimpr < current_residue->get_number_of_impropers(); ++nimpr) {
      Atoms as = current_residue->get_improper(nimpr).get_atoms(
                                   current_residue, previous_residue,
                                   next_residue, resmap);
      if (as.size() > 0) {
        try {
          const CHARMMDihedralParameters &p =
              ff->get_improper_parameters(CHARMMAtom(as[0]).get_charmm_type(),
                                          CHARMMAtom(as[1]).get_charmm_type(),
                                          CHARMMAtom(as[2]).get_charmm_type(),
                                          CHARMMAtom(as[3]).get_charmm_type());
            Dihedral id
              = Dihedral::setup_particle(new Particle(as[0]->get_model()),
                                         core::XYZ(as[0]), core::XYZ(as[1]),
                                         core::XYZ(as[2]), core::XYZ(as[3]));
            // CHARMM ideal value is in angles; convert to radians
            id.set_ideal(p.ideal / 180.0 * PI);
            id.set_multiplicity(p.multiplicity);
            id.set_stiffness(std::sqrt(p.force_constant * 2.0));
            ps.push_back(id);
        } catch (const IndexException &e) {
          // if no parameters, simply swallow the exception; do not
          // create an improper
        }
      }
    }
  }

  CHARMMResidueTopology *get_two_patch_residue(std::string &name,
                                               CHARMMResidueTopology *res1,
                                               CHARMMResidueTopology *res2)
  {
    if (name.size() > 2 && name[1] == ':') {
      if (name[0] == '1') {
        name.erase(0, 2);
        return res1;
      } else if (name[0] == '2') {
        name.erase(0, 2);
        return res2;
      }
    }
    IMP_THROW("Patching residue atom " << name
              << " does not start with 1: or 2:", ValueException);
  }

  std::pair<CHARMMResidueTopology *, CHARMMAtomTopology>
  handle_two_patch_atom(const CHARMMAtomTopology &atom,
                        CHARMMResidueTopology *res1,
                        CHARMMResidueTopology *res2)
  {
    std::string name = atom.get_name();
    CHARMMResidueTopology *res = get_two_patch_residue(name, res1, res2);
    if (res == res1) {
      return std::make_pair(res1, CHARMMAtomTopology(name, atom));
    } else {
      return std::make_pair(res2, CHARMMAtomTopology(name, atom));
    }
  }

  template <unsigned int D>
  CHARMMResidueTopology *get_two_patch_residue_for_bond(
                                 const CHARMMBond<D> &bond,
                                 CHARMMResidueTopology *res1,
                                 CHARMMResidueTopology *res2)
  {
    for (unsigned int i = 0; i < D; ++i) {
      std::string name = bond.get_endpoint(i).get_atom_name();
      CHARMMResidueTopology *res = get_two_patch_residue(name, res1, res2);
      if (res == res1) {
        // prefer to create bonds originating in the first residue, unless
        // all atoms in the bond live in the second residue
        return res1;
      }
    }
    return res2;
  }

  template <unsigned int D>
  CHARMMBond<D> handle_two_patch_bond(const CHARMMBond<D> &bond,
                                 CHARMMResidueTopology *res1,
                                 CHARMMResidueTopology *res2,
                                 CHARMMResidueTopology *first_res)
  {
    std::vector<CHARMMBondEndpoint> endpoints;
    for (unsigned int i = 0; i < D; ++i) {
      std::string name = bond.get_endpoint(i).get_atom_name();
      CHARMMResidueTopology *res = get_two_patch_residue(name, res1, res2);
      // Use new atom name (as modified by get_two_patch_residue) for the
      // endpoint; if the atom is not in first_res, keep a residue pointer
      // in the endpoint
      if (res == first_res) {
        endpoints.push_back(CHARMMBondEndpoint(name));
      } else {
        endpoints.push_back(CHARMMBondEndpoint(name, res2));
      }
    }
    return CHARMMBond<D>(endpoints);
  }

}


CHARMMBondEndpoint::CHARMMBondEndpoint(std::string atom_name,
                                       CHARMMResidueTopology *residue)
  : atom_name_(atom_name) {
  if (residue) {
    residue_=residue;
  }
}

  //! Map the endpoint to an Atom particle.
Atom CHARMMBondEndpoint::get_atom(const CHARMMResidueTopology *current_residue,
                const CHARMMResidueTopology *previous_residue,
                const CHARMMResidueTopology *next_residue,
                const std::map<const CHARMMResidueTopology *,
                               Hierarchy> &resmap) const {
    if (residue_) {
      CHARMMResidueTopology *res
        =dynamic_cast<CHARMMResidueTopology*>(residue_.get());
      return IMP::atom::get_atom(resmap.find(res)->second.get_as_residue(),
                                 AtomType(atom_name_));
    } else if (atom_name_[0] == '+') {
      if (next_residue) {
        return IMP::atom::get_atom(resmap.find(next_residue)->second.
                                                         get_as_residue(),
                                   AtomType(atom_name_.substr(1)));
      } else {
        return Atom();
      }
    } else if (atom_name_[0] == '-') {
      if (previous_residue) {
        return IMP::atom::get_atom(resmap.find(previous_residue)->second.
                                                           get_as_residue(),
                                   AtomType(atom_name_.substr(1)));
      } else {
        return Atom();
      }
    } else {
      return IMP::atom::get_atom(resmap.find(current_residue)->second.
                                                          get_as_residue(),
                                 AtomType(atom_name_));
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

void CHARMMIdealResidueTopology::remove_atom(std::string name)
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

void CHARMMPatch::apply(CHARMMResidueTopology *res) const
{
  if (res->get_patched()) {
    IMP_THROW("Cannot patch an already-patched residue", ValueException);
  }

  // Copy or update atoms
  for (std::vector<CHARMMAtomTopology>::const_iterator it = atoms_.begin();
       it != atoms_.end(); ++it) {
    try {
      res->get_atom(it->get_name()) = *it;
    } catch (ValueException &e) {
      res->add_atom(*it);
    }
  }

  // Delete atoms
  for (std::vector<std::string>::const_iterator it = deleted_atoms_.begin();
       it != deleted_atoms_.end(); ++it) {
    try {
      res->remove_atom(*it);
    } catch (ValueException &e) {
      // ignore atoms that don't exist to start with
    }
  }

  // Add angles/bonds/dihedrals/impropers
  for (unsigned int i = 0; i < get_number_of_bonds(); ++i) {
    res->add_bond(get_bond(i));
  }
  for (unsigned int i = 0; i < get_number_of_angles(); ++i) {
    res->add_angle(get_angle(i));
  }
  for (unsigned int i = 0; i < get_number_of_dihedrals(); ++i) {
    res->add_dihedral(get_dihedral(i));
  }
  for (unsigned int i = 0; i < get_number_of_impropers(); ++i) {
    res->add_improper(get_improper(i));
  }

  res->set_patched(true);
}

void CHARMMPatch::apply(CHARMMResidueTopology *res1,
                        CHARMMResidueTopology *res2) const
{
  if (res1->get_patched()) {
    IMP_THROW("Cannot patch an already-patched residue", ValueException);
  }
  if (res2->get_patched()) {
    IMP_THROW("Cannot patch an already-patched residue", ValueException);
  }

  // Copy or update atoms
  for (std::vector<CHARMMAtomTopology>::const_iterator it = atoms_.begin();
       it != atoms_.end(); ++it) {
    std::pair<CHARMMResidueTopology *, CHARMMAtomTopology> resatom =
                                handle_two_patch_atom(*it, res1, res2);
    try {
      resatom.first->get_atom(resatom.second.get_name()) = resatom.second;
    } catch (ValueException &e) {
      resatom.first->add_atom(resatom.second);
    }
  }

  // Delete atoms
  for (std::vector<std::string>::const_iterator it = deleted_atoms_.begin();
       it != deleted_atoms_.end(); ++it) {
    std::pair<CHARMMResidueTopology *, CHARMMAtomTopology> resatom =
                                handle_two_patch_atom(*it, res1, res2);
    try {
      resatom.first->remove_atom(resatom.second.get_name());
    } catch (ValueException &e) {
      // ignore atoms that don't exist to start with
    }
  }

  // Add angles/bonds/dihedrals/impropers
  for (unsigned int i = 0; i < get_number_of_bonds(); ++i) {
    CHARMMResidueTopology *res =
                get_two_patch_residue_for_bond(get_bond(i), res1, res2);
    res->add_bond(handle_two_patch_bond(get_bond(i), res1, res2, res));
  }
  for (unsigned int i = 0; i < get_number_of_angles(); ++i) {
    CHARMMResidueTopology *res =
               get_two_patch_residue_for_bond(get_angle(i), res1, res2);
    res->add_angle(handle_two_patch_bond(get_angle(i), res1, res2, res));
  }
  for (unsigned int i = 0; i < get_number_of_dihedrals(); ++i) {
    CHARMMResidueTopology *res =
                    get_two_patch_residue_for_bond(get_dihedral(i), res1, res2);
    res->add_dihedral(handle_two_patch_bond(get_dihedral(i), res1, res2, res));
  }
  for (unsigned int i = 0; i < get_number_of_impropers(); ++i) {
    CHARMMResidueTopology *res =
                    get_two_patch_residue_for_bond(get_improper(i), res1, res2);
    res->add_improper(handle_two_patch_bond(get_improper(i), res1, res2, res));
  }

  res1->set_patched(true);
  res2->set_patched(true);
}

void CHARMMResidueTopologyBase::do_show(std::ostream &) const
{
}

void CHARMMIdealResidueTopology::do_show(std::ostream &) const
{
}

void CHARMMPatch::do_show(std::ostream &) const
{
}

void CHARMMResidueTopology::do_show(std::ostream &) const
{
}

void CHARMMSegmentTopology::do_show(std::ostream &) const
{
}

void CHARMMSegmentTopology::apply_default_patches(const CHARMMParameters *ff)
{
  if (get_number_of_residues() == 0) return;

  CHARMMResidueTopology *first = get_residue(0);
  CHARMMResidueTopology *last = get_residue(get_number_of_residues() - 1);

  if (first->get_default_first_patch() != "") {
    ff->get_patch(first->get_default_first_patch())->apply(first);
  }

  if (last->get_default_first_patch() != "") {
    // If chain contains only a single residue, allow both the first and last
    // patch to be applied to it
    if (get_number_of_residues() == 1
        && first->get_default_first_patch() != "") {
      first->set_patched(false);
    }

    ff->get_patch(last->get_default_last_patch())->apply(last);
  }
}

void CHARMMTopology::do_show(std::ostream &) const
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
      if (!CHARMMAtom::particle_is_instance(*atit)) {
        try {
          CHARMMAtom::setup_particle(*atit,
                                     it->first->get_atom(typ)
                                     .get_charmm_type());
        } catch (ValueException &e) {
          IMP_WARN_ONCE("Could not determine CHARMM atom type for atom "
                        << typ << " in residue " << Residue(it->second)
                        << std::endl, warn_context_);
        }
      } else {
        // Override existing type if present
        CHARMMAtom(*atit).set_charmm_type(it->first->get_atom(typ)
                                          .get_charmm_type());
      }
    }
  }
  IMP_IF_LOG(VERBOSE) {
    warn_context_.dump_warnings();
  } else {
    warn_context_.clear_warnings();
  }
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
                      << typ << " in residue " << Residue(it->second)
                      << std::endl, warn_context_);
      }
    }
  }
  IMP_IF_LOG(VERBOSE) {
    warn_context_.dump_warnings();
  } else {
    warn_context_.clear_warnings();
  }
}

Particles CHARMMTopology::add_bonds(Hierarchy hierarchy,
                                    const CHARMMParameters *ff) const
{
  ResMap resmap;
  map_residue_topology_to_hierarchy(hierarchy, resmap);
  Particles ps;

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
      add_residue_bonds(cur, prev, next, resmap, ff, ps);
      prev = cur;
    }
  }
  return ps;
}

Particles CHARMMTopology::add_impropers(Hierarchy hierarchy,
                                        const CHARMMParameters *ff) const
{
  ResMap resmap;
  map_residue_topology_to_hierarchy(hierarchy, resmap);
  Particles ps;

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
      add_residue_impropers(cur, prev, next, resmap, ff, ps);
      prev = cur;
    }
  }
  return ps;
}

Hierarchy CHARMMTopology::create_hierarchy(Model *model) const
{
  char chain_id = 'A';
  Hierarchy root = Hierarchy::setup_particle(new Particle(model));
  for (CHARMMSegmentTopologyConstIterator segit = segments_begin();
       segit != segments_end(); ++segit) {
    const CHARMMSegmentTopology *seg = *segit;
    Chain chain = Chain::setup_particle(new Particle(model), chain_id++);
    root.add_child(chain);
    for (unsigned int nres = 0; nres < seg->get_number_of_residues(); ++nres) {
      const CHARMMResidueTopology *res = seg->get_residue(nres);
      ResidueType restyp = ResidueType(res->get_type());
      Residue residue = Residue::setup_particle(new Particle(model), restyp);
      chain.add_child(residue);
      for (unsigned int natm = 0; natm < res->get_number_of_atoms(); ++natm) {
        const CHARMMAtomTopology *atom = &res->get_atom(natm);
        AtomType atmtyp = AtomType(atom->get_name());
        Atom atm = Atom::setup_particle(new Particle(model), atmtyp);
        residue.add_child(atm);
      }
    }
  }
  return root;
}

IMP_LIST_IMPL(CHARMMSegmentTopology, CHARMMResidueTopology, residue,
              CHARMMResidueTopology *, CHARMMResidueTopologies,
              { obj->set_was_used(true); }, {}, {});

IMP_LIST_IMPL(CHARMMTopology, CHARMMSegmentTopology, segment,
              CHARMMSegmentTopology *, CHARMMSegmentTopologies,
              { obj->set_was_used(true); }, {}, {});

IMPATOM_END_NAMESPACE
