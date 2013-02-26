/**
 * \file charmm_topology.cpp \brief Classes for handling CHARMM-style topology.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/exception.h>
#include <IMP/constants.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/atom/charmm_topology.h>
#include <IMP/atom/charmm_segment_topology.h>
#include <IMP/atom/CHARMMParameters.h>
#include <IMP/atom/CHARMMAtom.h>
#include <IMP/atom/Charged.h>
#include <IMP/atom/angle_decorators.h>
#include <IMP/base/check_macros.h>
#include <IMP/base/log_macros.h>

#include <boost/algorithm/string.hpp>
#include <set>
#include <algorithm>

IMPATOM_BEGIN_NAMESPACE

namespace {
  // Convert a PDB-style atom name into CHARMM format
  std::string make_charmm_atom_name(std::string name) {
    std::string charmm_name = name;
    // Strip HET: prefix if present
    if (charmm_name.substr(0, 4) == "HET:") {
      charmm_name.erase(0, 4);
    }
    boost::trim(charmm_name); // remove all spaces
    return charmm_name;
  }

  class atom_has_name {
    std::string name_;
  public:
    atom_has_name(std::string name) : name_(make_charmm_atom_name(name)) {}
    bool operator()(const CHARMMAtomTopology &at) {
      return (at.get_name() == name_);
    }
  };

  template<unsigned int D>
  class bond_has_atom {
    std::string name_;
  public:
    bond_has_atom(std::string name) : name_(name) {}
    bool operator()(const CHARMMConnection<D> &bond) {
      return bond.get_contains_atom(name_);
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
          IMP_WARN(e.what());
        }
        ps.push_back(bd);
      }
    }
  }

  void add_residue_dihedrals(const CHARMMResidueTopology *current_residue,
                             const CHARMMResidueTopology *previous_residue,
                             const CHARMMResidueTopology *next_residue,
                             const std::map<const CHARMMResidueTopology *,
                                            Hierarchy> &resmap,
                             const CHARMMParameters *ff,
                             Particles &ps)
  {
    for (unsigned int ndih = 0;
         ndih < current_residue->get_number_of_dihedrals(); ++ndih) {
      Atoms as = current_residue->get_dihedral(ndih).get_atoms(
                                   current_residue, previous_residue,
                                   next_residue, resmap);
      if (as.size() > 0) {
        internal::add_dihedral_to_list(ff, as[0], as[1], as[2], as[3], ps);
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
        } catch (const IndexException &) {
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
    if (name.size() >= 2) {
      // Allow both for CHARMM-style 1CA atom naming and MODELLER-style 1:CA
      int to_erase = (name[1] == ':' ? 2 : 1);
      if (name[0] == '1') {
        name.erase(0, to_erase);
        return res1;
      } else if (name[0] == '2') {
        name.erase(0, to_erase);
        return res2;
      }
    }
    IMP_THROW("Patching residue atom " << name
              << " does not start with 1 or 2", ValueException);
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
                                 const CHARMMConnection<D> &bond,
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
  base::Vector<CHARMMBondEndpoint>
          handle_two_patch_bond(const CHARMMConnection<D> &bond,
                                CHARMMResidueTopology *res1,
                                CHARMMResidueTopology *res2,
                                CHARMMResidueTopology *first_res)
  {
    base::Vector<CHARMMBondEndpoint> endpoints;
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
    return endpoints;
  }

  Atom get_atom_by_name(Hierarchy h, const std::string atom_name) {
    Residue r = h.get_as_residue();
    if (r.get_is_protein() || r.get_is_rna() || r.get_is_dna()) {
      return IMP::atom::get_atom(r, AtomType(atom_name));
    } else {
      return IMP::atom::get_atom(r, AtomType("HET:" + atom_name));
    }
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
    return get_atom_by_name(resmap.find(res)->second, atom_name_);
  } else if (atom_name_[0] == '+') {
    if (next_residue) {
      return get_atom_by_name(resmap.find(next_residue)->second,
                              atom_name_.substr(1));
    } else {
      return Atom();
    }
  } else if (atom_name_[0] == '-') {
    if (previous_residue) {
      return get_atom_by_name(resmap.find(previous_residue)->second,
                              atom_name_.substr(1));
    } else {
      return Atom();
    }
  } else {
    return get_atom_by_name(resmap.find(current_residue)->second, atom_name_);
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
  base::Vector<CHARMMAtomTopology>::iterator it
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
  base::Vector<CHARMMAtomTopology>::const_iterator it
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
  base::Vector<CHARMMAtomTopology>::iterator it
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

namespace {
  void check_empty_patch(const CHARMMPatch *patch) {
    if (patch->get_number_of_atoms() == 0
        && patch->get_number_of_removed_atoms() == 0
        && patch->get_number_of_bonds() == 0
        && patch->get_number_of_angles() == 0
        && patch->get_number_of_dihedrals() == 0
        && patch->get_number_of_impropers() == 0
        && patch->get_number_of_internal_coordinates() == 0) {
      IMP_WARN(*patch << " appears to be empty - nothing done");
    }
  }
}

void CHARMMPatch::apply(CHARMMResidueTopology *res) const
{
  if (res->get_patched()) {
    IMP_THROW("Cannot patch an already-patched residue", ValueException);
  }
  check_empty_patch(this);

  // Copy or update atoms
  for (base::Vector<CHARMMAtomTopology>::const_iterator it = atoms_.begin();
       it != atoms_.end(); ++it) {
    try {
      res->get_atom(it->get_name()) = *it;
    } catch (ValueException &) {
      res->add_atom(*it);
    }
  }

  // Delete atoms
  for (base::Vector<std::string>::const_iterator it = deleted_atoms_.begin();
       it != deleted_atoms_.end(); ++it) {
    try {
      res->remove_atom(*it);
    } catch (ValueException &) {
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

  // Add internal coordinates
  for (unsigned int i = 0; i < get_number_of_internal_coordinates(); ++i) {
    res->add_internal_coordinate(get_internal_coordinate(i));
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
  check_empty_patch(this);

  // Extra checks for the commonly-used CHARMM DISU patch
  if (get_type() == "DISU"
      && (res1->get_type() != "CYS" || res2->get_type() != "CYS")) {
    IMP_WARN("Applying a DISU patch to two residues that are not both 'CYS' "
             "(they are " << *res1 << " and " << *res2 << "). This is "
             "probably not what was intended.");
  }

  // Copy or update atoms
  for (base::Vector<CHARMMAtomTopology>::const_iterator it = atoms_.begin();
       it != atoms_.end(); ++it) {
    std::pair<CHARMMResidueTopology *, CHARMMAtomTopology> resatom =
                                handle_two_patch_atom(*it, res1, res2);
    try {
      resatom.first->get_atom(resatom.second.get_name()) = resatom.second;
    } catch (ValueException &) {
      resatom.first->add_atom(resatom.second);
    }
  }

  // Delete atoms
  for (base::Vector<std::string>::const_iterator it = deleted_atoms_.begin();
       it != deleted_atoms_.end(); ++it) {
    std::pair<CHARMMResidueTopology *, CHARMMAtomTopology> resatom =
                                handle_two_patch_atom(*it, res1, res2);
    try {
      resatom.first->remove_atom(resatom.second.get_name());
    } catch (ValueException &) {
      // ignore atoms that don't exist to start with
    }
  }

  // Add angles/bonds/dihedrals/impropers
  for (unsigned int i = 0; i < get_number_of_bonds(); ++i) {
    CHARMMResidueTopology *res =
                get_two_patch_residue_for_bond(get_bond(i), res1, res2);
    res->add_bond(CHARMMBond(handle_two_patch_bond(get_bond(i), res1,
                                                   res2, res)));
  }
  for (unsigned int i = 0; i < get_number_of_angles(); ++i) {
    CHARMMResidueTopology *res =
               get_two_patch_residue_for_bond(get_angle(i), res1, res2);
    res->add_angle(CHARMMAngle(handle_two_patch_bond(get_angle(i), res1, res2,
                                                     res)));
  }
  for (unsigned int i = 0; i < get_number_of_dihedrals(); ++i) {
    CHARMMResidueTopology *res =
                    get_two_patch_residue_for_bond(get_dihedral(i), res1, res2);
    res->add_dihedral(CHARMMDihedral(handle_two_patch_bond(get_dihedral(i),
                                                           res1, res2, res)));
  }
  for (unsigned int i = 0; i < get_number_of_impropers(); ++i) {
    CHARMMResidueTopology *res =
                    get_two_patch_residue_for_bond(get_improper(i), res1, res2);
    res->add_improper(CHARMMDihedral(handle_two_patch_bond(get_improper(i),
                                                           res1, res2, res)));
  }

  // Add internal coordinates
  for (unsigned int i = 0; i < get_number_of_internal_coordinates(); ++i) {
    CHARMMInternalCoordinate ic = get_internal_coordinate(i);
    CHARMMResidueTopology *res =
                    get_two_patch_residue_for_bond(get_internal_coordinate(i),
                                                   res1, res2);
    res->add_internal_coordinate(
            CHARMMInternalCoordinate(handle_two_patch_bond(ic, res1,
                                                           res2, res),
                                ic.get_first_distance(), ic.get_first_angle(),
                                ic.get_dihedral(), ic.get_second_angle(),
                                ic.get_second_distance(), ic.get_improper()));
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

  if (last->get_default_last_patch() != "") {
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

void CHARMMTopology::add_sequence(std::string sequence)
{
  IMP_NEW(CHARMMSegmentTopology, seg, ());

  for (std::string::const_iterator it = sequence.begin();
       it != sequence.end(); ++it) {
    if (*it == '/') {
      add_segment(seg);
      seg = new CHARMMSegmentTopology();
    } else {
      ResidueType restyp = get_residue_type(*it);
      IMP_NEW(CHARMMResidueTopology, res,
              (force_field_->get_residue_topology(restyp)));
      seg->add_residue(res);
    }
  }
  add_segment(seg);
}

void CHARMMTopology::map_residue_topology_to_hierarchy(Hierarchy hierarchy,
                                                       ResMap &resmap) const
{
  Hierarchies chains = get_by_type(hierarchy, CHAIN_TYPE);
  IMP_USAGE_CHECK(chains.size() == get_number_of_segments(),
                  "Hierarchy does not match topology");

  unsigned int nseg = 0;
  for (Hierarchies::iterator chainit = chains.begin();
       chainit != chains.end(); ++chainit, ++nseg) {
    CHARMMSegmentTopology *topseg = get_segment(nseg);
    Hierarchies residues = get_by_type(*chainit, RESIDUE_TYPE);
    IMP_USAGE_CHECK(residues.size() == topseg->get_number_of_residues(),
                    "Hierarchy does not match topology");
    unsigned int nres = 0;
    for (Hierarchies::iterator resit = residues.begin();
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
    Hierarchies atoms = get_by_type(it->second, ATOM_TYPE);
    for (Hierarchies::iterator atit = atoms.begin(); atit != atoms.end();
         ++atit) {
      AtomType typ = Atom(*atit).get_atom_type();
      if (!CHARMMAtom::particle_is_instance(*atit)) {
        try {
          CHARMMAtom::setup_particle(*atit,
                                     it->first->get_atom(typ)
                                     .get_charmm_type());
        } catch (ValueException &) {
          IMP_WARN_ONCE(typ.get_string()
                        +Residue(it->second).get_residue_type().get_string(),
                        "Could not determine CHARMM atom type for atom "
                        << typ << " in residue " << Residue(it->second),
                        warn_context_);
        }
      } else {
        // Override existing type if present
        CHARMMAtom(*atit).set_charmm_type(it->first->get_atom(typ)
                                          .get_charmm_type());
      }
    }
  }
  bool dumped=false;
  IMP_IF_LOG(VERBOSE) {
    dumped=true;
    warn_context_.dump_warnings();
  }
  if (!dumped) {
    warn_context_.clear_warnings();
  }
}

void CHARMMTopology::add_missing_atoms(Hierarchy hierarchy) const
{
  Model *model = hierarchy.get_particle()->get_model();
  ResMap resmap;
  map_residue_topology_to_hierarchy(hierarchy, resmap);

  for (ResMap::iterator it = resmap.begin(); it != resmap.end(); ++it) {

    // Get atoms currently in this residue
    Hierarchies h = get_by_type(it->second, ATOM_TYPE);
    std::set<std::string> existing_atoms;
    for (Hierarchies::iterator atit = h.begin(); atit != h.end(); ++atit) {
      AtomType typ = Atom(*atit).get_atom_type();
      existing_atoms.insert(make_charmm_atom_name(typ.get_string()));
    }

    Residue r = it->second.get_as_residue();
    bool is_ligand = !(r.get_is_protein() || r.get_is_rna() || r.get_is_dna());

    // Look at all atoms in the topology; add any that aren't in existing_atoms
    for (unsigned int i = 0; i < it->first->get_number_of_atoms(); ++i) {
      const CHARMMAtomTopology &atomtop = it->first->get_atom(i);
      if (existing_atoms.find(atomtop.get_name()) == existing_atoms.end()) {
        std::string name = atomtop.get_name();
        if (is_ligand) {
          name = "HET:" + name;
        }
        AtomType typ = AtomType(name);
        Atom atm = Atom::setup_particle(new Particle(model), typ);
        CHARMMAtom::setup_particle(atm, atomtop.get_charmm_type());
        it->second.add_child(atm);
      }
    }
  }
}

namespace {
  struct ModelInternalCoordinate {
    float first_distance, second_distance, first_angle, second_angle, dihedral;
    bool improper;
    Atoms atoms;
    ModelInternalCoordinate(const CHARMMInternalCoordinate &ic,
                            Atoms new_atoms) : atoms(new_atoms) {
      first_distance = ic.get_first_distance();
      second_distance = ic.get_second_distance();
      first_angle = ic.get_first_angle();
      second_angle = ic.get_second_angle();
      dihedral = ic.get_dihedral();
      improper = ic.get_improper();
    }

    // Get the distance between the two atoms stated by this internal
    // coordinate. If no such distance is present, return 0.
    float get_distance(Atom i, Atom j) const {
      if (!improper && ((i == atoms[0] && j == atoms[1])
                        || (i == atoms[1] && j == atoms[0]))) {
        return first_distance;
      } else if (improper && ((i == atoms[0] && j == atoms[2])
                              || (i == atoms[2] && j == atoms[0]))) {
        return first_distance;
      } else if ((i == atoms[2] && j == atoms[3])
                 || (i == atoms[3] && j == atoms[2])) {
        return second_distance;
      }
      return 0.;
    }

    // Get the angle between the three atoms stated by this internal
    // coordinate. If no such angle is present, return 0.
    float get_angle(Atom i, Atom j, Atom k) const {
      if (!improper && j == atoms[1] && ((i == atoms[0] && k == atoms[2])
                                         || (i == atoms[2] && k == atoms[0]))) {
        return first_angle;
      } else if (improper && j == atoms[2]
                 && ((i == atoms[0] && k == atoms[1])
                     || (i == atoms[1] && k == atoms[0]))) {
        return first_angle;
      } else if (j == atoms[2] && ((i == atoms[1] && k == atoms[3])
                                   || (i == atoms[3] && k == atoms[1]))) {
        return second_angle;
      }
      return 0.;
    }
  };

  void build_internal_coordinates(const CHARMMSegmentTopology *seg,
                             const std::map<const CHARMMResidueTopology *,
                                            Hierarchy> &resmap,
                             base::Vector<ModelInternalCoordinate> &ics) {
    const CHARMMResidueTopology *prev = nullptr;
    for (unsigned int nres = 0; nres < seg->get_number_of_residues(); ++nres) {
      const CHARMMResidueTopology *cur = seg->get_residue(nres);
      const CHARMMResidueTopology *next =
               nres < seg->get_number_of_residues() - 1 ?
               seg->get_residue(nres + 1) : nullptr;
      for (unsigned int nic = 0;
           nic < cur->get_number_of_internal_coordinates(); ++nic) {
        const CHARMMInternalCoordinate &ic =
                     cur->get_internal_coordinate(nic);
        Atoms atoms = ic.get_atoms(cur, prev, next, resmap);
        if (atoms.size() > 0) {
          ics.push_back(ModelInternalCoordinate(ic, atoms));
        }
      }
      prev = cur;
    }
  }

  float fill_distance(Atom i, Atom j, const CHARMMParameters *ff) {
    if (CHARMMAtom::particle_is_instance(i)
        && CHARMMAtom::particle_is_instance(j)) {
      try {
        return ff->get_bond_parameters(CHARMMAtom(i).get_charmm_type(),
                                       CHARMMAtom(j).get_charmm_type()).ideal;
      } catch (IndexException &) {
      }
    }
    return 0.;
  }

  float fill_angle(Atom i, Atom j, Atom k, const CHARMMParameters *ff) {
    if (CHARMMAtom::particle_is_instance(i)
        && CHARMMAtom::particle_is_instance(j)
        && CHARMMAtom::particle_is_instance(k)) {
      try {
        return ff->get_angle_parameters(CHARMMAtom(i).get_charmm_type(),
                                        CHARMMAtom(j).get_charmm_type(),
                                        CHARMMAtom(k).get_charmm_type()).ideal;
      } catch (IndexException &) {
      }
    }
    return 0.;
  }

  // CHARMM format allows for distances or angles (but not dihedrals) to
  // be zero; fill in these missing values using atom types and
  // parameter file information if available.
  void fill_internal_coordinates(base::Vector<ModelInternalCoordinate> &ics,
                                 const CHARMMParameters *ff) {
    for (base::Vector<ModelInternalCoordinate>::iterator it = ics.begin();
         it != ics.end(); ++it) {
      if (it->first_distance == 0.) {
        if (it->improper) {
          it->first_distance = fill_distance(it->atoms[0], it->atoms[2], ff);
        } else {
          it->first_distance = fill_distance(it->atoms[0], it->atoms[1], ff);
        }
      }
      if (it->second_distance == 0.) {
        it->second_distance = fill_distance(it->atoms[2], it->atoms[3], ff);
      }

      if (it->first_angle == 0.) {
        if (it->improper) {
          it->first_angle = fill_angle(it->atoms[0], it->atoms[2], it->atoms[1],
                                       ff);
        } else {
          it->first_angle = fill_angle(it->atoms[0], it->atoms[1], it->atoms[2],
                                       ff);
        }
      }
      if (it->second_angle == 0.) {
        it->second_angle = fill_angle(it->atoms[1], it->atoms[2], it->atoms[3],
                                      ff);
      }
    }
  }

  void build_cartesian(Atom known1, Atom known2, Atom known3, Atom unknown,
                       float r, float phi, float theta) {
    // Convert to radians
    theta = theta * PI / 180.;
    phi = phi * PI / 180.;

    double cost = std::cos(theta);
    double sint = std::sin(theta);
    double cosp = std::cos(phi);
    double sinp = std::sin(phi);

    algebra::Vector3D v1 = core::XYZ(known1).get_coordinates();
    algebra::Vector3D v2 = core::XYZ(known2).get_coordinates();
    algebra::Vector3D v3 = core::XYZ(known3).get_coordinates();

    algebra::Vector3D rjk = v2 - v3;
    algebra::Vector3D rjk_unit = rjk.get_unit_vector();
    algebra::Vector3D rij = v1 - v2;

    algebra::Vector3D cross = algebra::get_vector_product(rij,
                                                rjk_unit).get_unit_vector();
    algebra::Vector3D cross2 = algebra::get_vector_product(rjk_unit, cross);

    algebra::Vector3D wt(r * cost, r * sint * cosp, r * sint * sinp);

    algebra::Vector3D newc(wt[0] * rjk_unit[0] + wt[1] * cross2[0]
                           + wt[2] * cross[0],
                           wt[0] * rjk_unit[1] + wt[1] * cross2[1]
                           + wt[2] * cross[1],
                           wt[0] * rjk_unit[2] + wt[1] * cross2[2]
                           + wt[2] * cross[2]);
    core::XYZ::setup_particle(unknown, newc + v3);
  }

  // If exactly 3 out of the 4 atoms in the given internal coordinate have
  // defined Cartesian coordinates, and the internal coordinate has defined
  // distances and angles, build the Cartesian coordinates of the
  // remaining atom and return true. Otherwise, return false.
  bool build_cartesian_from_internal(const ModelInternalCoordinate &ic) {
    if (core::XYZ::particle_is_instance(ic.atoms[1])
        && core::XYZ::particle_is_instance(ic.atoms[2])) {
      if (!core::XYZ::particle_is_instance(ic.atoms[3])
          && core::XYZ::particle_is_instance(ic.atoms[0])) {
        float phi = ic.dihedral;
        float r = ic.second_distance;
        float theta = ic.second_angle;
        if (r != 0. && theta != 0.) {
          build_cartesian(ic.atoms[0], ic.atoms[1],
                          ic.atoms[2], ic.atoms[3], r, phi, theta);
          return true;
        }
      } else if (!core::XYZ::particle_is_instance(ic.atoms[0])
                 && core::XYZ::particle_is_instance(ic.atoms[3])) {
        float phi = ic.dihedral;
        float r = ic.first_distance;
        float theta = ic.first_angle;
        if (r != 0. && theta != 0.) {
          if (ic.improper) {
            build_cartesian(ic.atoms[3], ic.atoms[1],
                            ic.atoms[2], ic.atoms[0], r, -phi, theta);
          } else {
            build_cartesian(ic.atoms[3], ic.atoms[2],
                            ic.atoms[1], ic.atoms[0], r, phi, theta);
          }
          return true;
        }
      }
    }
    return false;
  }

  unsigned build_cartesians_from_internal(
                             base::Vector<ModelInternalCoordinate> &ics) {
    base::Vector<ModelInternalCoordinate>::iterator newend =
         std::remove_if(ics.begin(), ics.end(), build_cartesian_from_internal);
    unsigned numbuilt = ics.end() - newend;
    // Any internal coordinate used to build Cartesian coordinates must
    // now have all 4 of its atoms with defined coordinates, so is no longer
    // informative and can be removed to avoid slowing down further runs.
    ics.erase(newend, ics.end());
    return numbuilt;
  }

  bool seed_triplet(Atom i, Atom j, Atom k,
                    const base::Vector<ModelInternalCoordinate> &ics,
                    const algebra::Vector3D &seed) {
    double rij = 0., rjk = 0., tijk = 0.;
    for (base::Vector<ModelInternalCoordinate>::const_iterator it = ics.begin();
         it != ics.end() && (rij == 0. || rjk == 0. || tijk == 0.); ++it) {
      if (rij == 0.) {
        rij = it->get_distance(i, j);
      }
      if (rjk == 0.) {
        rjk = it->get_distance(j, k);
      }
      if (tijk == 0.) {
        tijk = it->get_angle(i, j, k);
      }
    }
    if (rij == 0. || rjk == 0. || tijk == 0.) {
      return false;
    } else {
      // Convert from degrees to radians
      tijk = tijk * PI / 180.;
      core::XYZ::setup_particle(i, seed);
      core::XYZ::setup_particle(j, seed + algebra::Vector3D(rij, 0., 0.));
      core::XYZ::setup_particle(k, seed
                                + algebra::Vector3D(rij - rjk * std::cos(tijk),
                                                    rjk * std::sin(tijk), 0.));
      return true;
    }
  }

  // Find three atoms i,j,k for which Rij, Rjk and the angle Tijk are
  // given by internal coordinates, and set their Cartesian coordinates.
  // The first atom is placed at the seed, the second Rij along the x axis,
  // and the third at an angle Tijk on the xy plane, Rjk from the second.
  // Returns true only if a triplet was found.
  bool seed_coordinates(const base::Vector<ModelInternalCoordinate> &ics,
                        const algebra::Vector3D &seed) {
    for (base::Vector<ModelInternalCoordinate>::const_iterator it = ics.begin();
         it != ics.end(); ++it) {
      if (!it->improper) {
        if (seed_triplet(it->atoms[0], it->atoms[1], it->atoms[2], ics, seed)
            || seed_triplet(it->atoms[1], it->atoms[2], it->atoms[3],
                            ics, seed)) {
          return true;
        }
      }
    }
    return false;
  }

  unsigned count_atoms_with_coordinates(const CHARMMSegmentTopology *seg,
                const std::map<const CHARMMResidueTopology *,
                               Hierarchy> &resmap) {
    unsigned int ct = 0;
    for (CHARMMResidueTopologies::const_iterator it = seg->residues_begin();
         it != seg->residues_end(); ++it) {
      Hierarchy h = resmap.find(*it)->second;
      for (unsigned int i = 0; i < h.get_number_of_children(); ++i) {
        Hierarchy child = h.get_child(i);
        if (child.get_as_atom() && core::XYZ::particle_is_instance(child)) {
          ++ct;
        }
      }
    }
    return ct;
  }

  unsigned int assign_remaining_coordinates(const CHARMMSegmentTopology *seg,
                         const std::map<const CHARMMResidueTopology *,
                                        Hierarchy> &resmap,
                         algebra::Vector3D &seed) {
    unsigned int assigned = 0;
    // visit all atoms; if defined, update seed; if undefined, set
    // coordinates as random offset from seed
    for (CHARMMResidueTopologies::const_iterator it = seg->residues_begin();
         it != seg->residues_end(); ++it) {
      Hierarchy h = resmap.find(*it)->second;
      for (unsigned int i = 0; i < h.get_number_of_children(); ++i) {
        Hierarchy child = h.get_child(i);
        if (child.get_as_atom()) {
          if (core::XYZ::particle_is_instance(child)) {
            seed = core::XYZ(child).get_coordinates();
          } else {
            ++assigned;
            algebra::Sphere3D sphere(seed, 0.5);
            core::XYZ::setup_particle(child,
                                      algebra::get_random_vector_in(sphere));
          }
        }
      }
    }
    return assigned;
  }
}

void CHARMMTopology::add_coordinates(Hierarchy hierarchy) const
{
  ResMap resmap;
  map_residue_topology_to_hierarchy(hierarchy, resmap);

  algebra::Vector3D seed(0., 0., 0.);
  for (CHARMMSegmentTopologyConstIterator segit = segments_begin();
       segit != segments_end(); ++segit) {
    base::Vector<ModelInternalCoordinate> ics;
    build_internal_coordinates(*segit, resmap, ics);

    fill_internal_coordinates(ics, force_field_);

    // If no atoms currently have Cartesian coordinates, place a triplet
    // of atoms near the seed position, so we can (hopefully) fill in the
    // rest from internal coordinates
    if (count_atoms_with_coordinates(*segit, resmap) == 0) {
      seed_coordinates(ics, seed);
    }

    // If we added at least one Cartesian coordinate, run again - there may now
    // be more coordinates we can fill in using the newly-assigned coordinates.
    while (build_cartesians_from_internal(ics) > 0) {}

    unsigned int nremain = assign_remaining_coordinates(*segit, resmap, seed);
    if (nremain > 0) {
      IMP_WARN("The Cartesian coordinates of " << nremain
               << " atoms could not be assigned from internal coordinates, "
                  "and so were assigned randomly to lie near atoms close "
                  "in sequence.");
    }

    // offset seed for start of next segment
    seed += algebra::Vector3D(2., 2., 2.);
  }
}

void CHARMMTopology::add_charges(Hierarchy hierarchy) const
{
  ResMap resmap;
  map_residue_topology_to_hierarchy(hierarchy, resmap);

  for (ResMap::iterator it = resmap.begin(); it != resmap.end(); ++it) {
    Hierarchies atoms = get_by_type(it->second, ATOM_TYPE);
    for (Hierarchies::iterator atit = atoms.begin(); atit != atoms.end();
         ++atit) {
      AtomType typ = Atom(*atit).get_atom_type();
      try {
        Charged::setup_particle(*atit,
                                it->first->get_atom(typ).get_charge());
      } catch (ValueException &) {
        IMP_WARN_ONCE(typ.get_string(), "Could not determine charge for atom "
                      << typ << " in residue " << Residue(it->second),
                      warn_context_);
      }
    }
  }
  // keep clang happy
  bool dumped=false;
  IMP_IF_LOG(VERBOSE) {
    dumped=true;
    warn_context_.dump_warnings();
  }
  if (!dumped) {
    warn_context_.clear_warnings();
  }
}

Particles CHARMMTopology::add_bonds(Hierarchy hierarchy) const
{
  ResMap resmap;
  map_residue_topology_to_hierarchy(hierarchy, resmap);
  Particles ps;

  for (CHARMMSegmentTopologyConstIterator segit = segments_begin();
       segit != segments_end(); ++segit) {
    const CHARMMSegmentTopology *seg = *segit;
    const CHARMMResidueTopology *prev = nullptr;
    for (unsigned int nres = 0; nres < seg->get_number_of_residues();
         ++nres) {
      const CHARMMResidueTopology *cur = seg->get_residue(nres);
      const CHARMMResidueTopology *next =
               nres < seg->get_number_of_residues() - 1 ?
               seg->get_residue(nres + 1) : nullptr;
      add_residue_bonds(cur, prev, next, resmap, force_field_, ps);
      prev = cur;
    }
  }
  return ps;
}

Particles CHARMMTopology::add_impropers(Hierarchy hierarchy) const
{
  ResMap resmap;
  map_residue_topology_to_hierarchy(hierarchy, resmap);
  Particles ps;

  for (CHARMMSegmentTopologyConstIterator segit = segments_begin();
       segit != segments_end(); ++segit) {
    const CHARMMSegmentTopology *seg = *segit;
    const CHARMMResidueTopology *prev = nullptr;
    for (unsigned int nres = 0; nres < seg->get_number_of_residues();
         ++nres) {
      const CHARMMResidueTopology *cur = seg->get_residue(nres);
      const CHARMMResidueTopology *next =
               nres < seg->get_number_of_residues() - 1 ?
               seg->get_residue(nres + 1) : nullptr;
      add_residue_impropers(cur, prev, next, resmap, force_field_, ps);
      prev = cur;
    }
  }
  return ps;
}

Particles CHARMMTopology::add_dihedrals(Hierarchy hierarchy) const
{
  ResMap resmap;
  map_residue_topology_to_hierarchy(hierarchy, resmap);
  Particles ps;

  for (CHARMMSegmentTopologyConstIterator segit = segments_begin();
       segit != segments_end(); ++segit) {
    const CHARMMSegmentTopology *seg = *segit;
    const CHARMMResidueTopology *prev = nullptr;
    for (unsigned int nres = 0; nres < seg->get_number_of_residues();
         ++nres) {
      const CHARMMResidueTopology *cur = seg->get_residue(nres);
      const CHARMMResidueTopology *next =
               nres < seg->get_number_of_residues() - 1 ?
               seg->get_residue(nres + 1) : nullptr;
      add_residue_dihedrals(cur, prev, next, resmap, force_field_, ps);
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
    int residue_index = 1;
    const CHARMMSegmentTopology *seg = *segit;
    Chain chain = Chain::setup_particle(new Particle(model), chain_id++);
    root.add_child(chain);
    for (unsigned int nres = 0; nres < seg->get_number_of_residues(); ++nres) {
      const CHARMMResidueTopology *res = seg->get_residue(nres);
      ResidueType restyp = ResidueType(res->get_type());
      Residue residue = Residue::setup_particle(new Particle(model), restyp,
                                                residue_index++);
      chain.add_child(residue);
      bool is_ligand = !(residue.get_is_protein() || residue.get_is_rna()
                         || residue.get_is_dna());
      for (unsigned int natm = 0; natm < res->get_number_of_atoms(); ++natm) {
        const CHARMMAtomTopology *atom = &res->get_atom(natm);
        std::string name = atom->get_name();
        if (is_ligand) {
          name = "HET:" + name;
        }
        AtomType atmtyp = AtomType(name);
        Atom atm = Atom::setup_particle(new Particle(model), atmtyp);
        residue.add_child(atm);
      }
    }
  }
  return root;
}

void CHARMMTopology::setup_hierarchy(Hierarchy hierarchy) const
{
  add_atom_types(hierarchy);
  add_missing_atoms(hierarchy);
  remove_charmm_untyped_atoms(hierarchy);
  add_coordinates(hierarchy);
}

IMP_LIST_IMPL(CHARMMSegmentTopology, CHARMMResidueTopology, residue,
              CHARMMResidueTopology *, CHARMMResidueTopologies);

IMP_LIST_IMPL(CHARMMTopology, CHARMMSegmentTopology, segment,
              CHARMMSegmentTopology *, CHARMMSegmentTopologies);

IMPATOM_END_NAMESPACE
