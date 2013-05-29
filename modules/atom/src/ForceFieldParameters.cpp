/**
 * \file ForceFieldParameters \brief
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/ForceFieldParameters.h>
#include <IMP/core/XYZR.h>
#include <IMP/Particle.h>
#include <IMP/atom/CHARMMParameters.h>
#include <IMP/atom/LennardJones.h>

IMPATOM_BEGIN_NAMESPACE

Float ForceFieldParameters::get_radius(Atom atom) const {
  String force_field_atom_type = get_force_field_atom_type(atom);
  return get_radius(force_field_atom_type);
}

Float ForceFieldParameters::get_epsilon(Atom atom) const {
  String force_field_atom_type = get_force_field_atom_type(atom);
  return get_epsilon(force_field_atom_type);
}

void ForceFieldParameters::add_radii(Hierarchy mhd, double scale,
                                     FloatKey radius_key) const {
  ParticlesTemp ps = get_by_type(mhd, ATOM_TYPE);
  for (unsigned int i = 0; i < ps.size(); i++) {
    Float radius = get_radius(Atom(ps[i])) * scale;
    if (ps[i]->has_attribute(radius_key)) {
      ps[i]->set_value(radius_key, radius);
    } else {
      ps[i]->add_attribute(radius_key, radius);
    }
  }
  // TODO: handle N-term and C-term
  warn_context_.dump_warnings();
}

void ForceFieldParameters::add_well_depths(Hierarchy mhd) const {
  ParticlesTemp ps = get_by_type(mhd, ATOM_TYPE);
  for (unsigned int i = 0; i < ps.size(); i++) {
    Float epsilon = get_epsilon(Atom(ps[i]));
    LennardJones::setup_particle(ps[i], -epsilon);
  }
  warn_context_.dump_warnings();
}

Float ForceFieldParameters::get_radius(
    const String& force_field_atom_type) const {
  if (force_field_atom_type.length() > 0 &&
      force_field_2_vdW_.find(force_field_atom_type) !=
          force_field_2_vdW_.end()) {
    return force_field_2_vdW_.find(force_field_atom_type)->second.second;
  }
  if (!force_field_atom_type.empty()) {
    IMP_WARN_ONCE(force_field_atom_type,
                  "Radius not found for type, default value is used \""
                      << force_field_atom_type << "\"" << std::endl,
                  warn_context_);
  }
  return 1.7;  // SOME DEFAULT VALUE!!
}

Float ForceFieldParameters::get_epsilon(
    const String& force_field_atom_type) const {
  if (force_field_atom_type.length() > 0 &&
      force_field_2_vdW_.find(force_field_atom_type) !=
          force_field_2_vdW_.end()) {
    return force_field_2_vdW_.find(force_field_atom_type)->second.first;
  }
  if (!force_field_atom_type.empty()) {
    IMP_WARN("Epsilon not found \"" << force_field_atom_type << "\""
                                    << std::endl);
  }
  return -0.1;  // SOME DEFAULT VALUE!!
}

void ForceFieldParameters::add_bonds(Hierarchy mhd) const {
  Hierarchies rs(get_by_type(mhd, RESIDUE_TYPE));
  for (unsigned int i = 0; i < rs.size(); ++i) {
    add_bonds(rs[i].get_as_residue());
    Hierarchy rn = get_next_residue(rs[i].get_as_residue());
    if (rn) {
      add_bonds(rs[i].get_as_residue(), rn.get_as_residue());
    }
  }
  warn_context_.dump_warnings();
}

void ForceFieldParameters::add_bonds(Residue rd1, Residue rd2) const {
  if (!rd1 || !rd2) return;  // if they aren't value residues.
  Atom ad1, ad2;
  // connect two residues by C-N bond
  if (rd1.get_is_protein() && rd2.get_is_protein()) {
    ad1 = get_atom(rd1, atom::AT_C);
    ad2 = get_atom(rd2, atom::AT_N);
  }
  // connect two nucleic acids by O3'-P bond
  if ((rd1.get_is_dna() && rd2.get_is_dna()) ||
      (rd1.get_is_rna() && rd2.get_is_rna())) {
    ad1 = get_atom(rd1, atom::AT_O3p);
    ad2 = get_atom(rd2, atom::AT_P);
  }
  if (!ad1 || !ad2) {
    IMP_WARN("Residues incomplete: " << rd1 << " and " << rd2 << std::endl);
    return;
  }
  Particle* p1 = ad1.get_particle();
  Particle* p2 = ad2.get_particle();

  Bonded b1, b2;
  if (Bonded::particle_is_instance(p1))
    b1 = Bonded::decorate_particle(p1);
  else
    b1 = Bonded::setup_particle(p1);

  if (Bonded::particle_is_instance(p2))
    b2 = Bonded::decorate_particle(p2);
  else
    b2 = Bonded::setup_particle(p2);

  create_bond(b1, b2, IMP::atom::Bond::SINGLE);
}

void ForceFieldParameters::add_bonds(Residue rd) const {
  if (!rd) return;
  ResidueType type = rd.get_residue_type();
  if (residue_bonds_.find(type) == residue_bonds_.end()) return;

  const base::Vector<Bond>& bonds = residue_bonds_.find(type)->second;
  for (unsigned int i = 0; i < bonds.size(); i++) {
    Atom ad1 = get_atom(rd, bonds[i].type1_);
    Atom ad2 = get_atom(rd, bonds[i].type2_);
    if (!ad1 || !ad2) {
      IMP_WARN_ONCE(
          rd.get_residue_type().get_string() + bonds[i].type1_.get_string() +
              bonds[i].type2_.get_string(),
          "In residue " << rd << " could not find atom " << bonds[i].type1_
                        << " or " << bonds[i].type2_,
          warn_context_);
      continue;
    }

    Particle* p1 = ad1.get_particle();
    Particle* p2 = ad2.get_particle();

    Bonded b1, b2;
    if (Bonded::particle_is_instance(p1))
      b1 = Bonded::decorate_particle(p1);
    else
      b1 = Bonded::setup_particle(p1);

    if (Bonded::particle_is_instance(p2))
      b2 = Bonded::decorate_particle(p2);
    else
      b2 = Bonded::setup_particle(p2);

    create_bond(b1, b2, bonds[i].bond_type_);
  }
}

String ForceFieldParameters::get_force_field_atom_type(Atom atom) const {
  AtomType atom_type = atom.get_atom_type();
  ResidueType residue_type = get_residue(atom).get_residue_type();
  static String empty_atom_type;
  if (atom_res_type_2_force_field_atom_type_.find(residue_type) ==
      atom_res_type_2_force_field_atom_type_.end()) {
    IMP_WARN_ONCE(residue_type.get_string(),
                  "Residue not found " << residue_type << std::endl,
                  warn_context_);
    return empty_atom_type;
  }

  //  string fixedAtomType = fixAtomType(atomName, residueName, NORM);
  const AtomTypeMap& atom_map =
      atom_res_type_2_force_field_atom_type_.find(residue_type)->second;
  if (atom_map.find(atom_type) == atom_map.end()) {
    IMP_WARN_ONCE(atom_type.get_string() + residue_type.get_string(),
                  "Atom not found " << atom_type << " residue " << residue_type
                                    << std::endl,
                  warn_context_);
    return empty_atom_type;
  }

  return atom_map.find(atom_type)->second.first;
}

IMPATOM_END_NAMESPACE
