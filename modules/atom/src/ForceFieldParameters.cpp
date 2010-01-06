/**
 * \file ForceFieldParameters \brief
 *
 * Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/atom/ForceFieldParameters.h>
#include <IMP/core/XYZR.h>
#include <IMP/Particle.h>
#include <IMP/atom/CharmmParameters.h>

IMPATOM_BEGIN_NAMESPACE

Float ForceFieldParameters::get_radius(AtomType atom_type,
                                       ResidueType residue_type) const
{
  String force_field_atom_type =
    get_force_field_atom_type(atom_type, residue_type);
  return get_radius(force_field_atom_type);
}

Float ForceFieldParameters::get_epsilon(AtomType atom_type,
                                        ResidueType residue_type) const
{
  String force_field_atom_type =
    get_force_field_atom_type(atom_type, residue_type);
  return get_epsilon(force_field_atom_type);
}

void ForceFieldParameters::add_radii(Hierarchy mhd, FloatKey radius_key) const
{
  Particles ps = get_by_type(mhd, ATOM_TYPE);
  for(unsigned int i=0; i<ps.size(); i++) {
    Float radius = get_radius(Atom(ps[i]).get_atom_type(),
                              get_residue(Atom(ps[i])).get_residue_type());
    core::XYZR::setup_particle(ps[i], radius, radius_key);
  }
  // TODO: handle N-term and C-term
  warn_context_.dump_warnings();
}

Float ForceFieldParameters::get_radius(
                           const String& force_field_atom_type) const
{
  if(force_field_atom_type.length() > 0 &&
     force_field_2_vdW_.find(force_field_atom_type) !=
     force_field_2_vdW_.end()) {
    return force_field_2_vdW_.find(force_field_atom_type)->second.second;
  }
  IMP_WARN_ONCE("Radius not found for type \""
                << force_field_atom_type << "\"" << std::endl, warn_context_);
  return 0;
}

Float ForceFieldParameters::get_epsilon(
                           const String& force_field_atom_type) const
{
  if(force_field_atom_type.length() > 0 &&
     force_field_2_vdW_.find(force_field_atom_type) !=
     force_field_2_vdW_.end()) {
    return force_field_2_vdW_.find(force_field_atom_type)->second.first;
  }
  IMP_WARN("Epsilon not found \"" << force_field_atom_type << "\""
           << std::endl);
  return -0.1; // SOME DEFAULT VALUE!!
}


void ForceFieldParameters::add_bonds(Hierarchy mhd) const {
  Hierarchies rs(get_by_type(mhd, RESIDUE_TYPE));
  for (unsigned int i=0;i < rs.size(); ++i) {
    add_bonds(rs[i].get_as_residue());
    Hierarchy rn= get_next_residue(rs[i].get_as_residue());
    if (rn) {
      add_bonds(rs[i].get_as_residue(), rn.get_as_residue());
    }
  }
  warn_context_.dump_warnings();
}


void ForceFieldParameters::add_bonds(Residue rd1, Residue rd2) const {
  if (!rd1 || !rd2) return; // if they aren't value residues.
  Atom ad1, ad2;
  // connect two residues by C-N bond
  if(rd1.get_is_protein()
     && rd2.get_is_protein()) {
    ad1 = get_atom(rd1, atom::AT_C);
    ad2 = get_atom(rd2, atom::AT_N);
   }
  // connect two nucleic acids by O3'-P bond
  if((rd1.get_is_dna() &&
      rd2.get_is_dna())
     || (rd1.get_is_rna() &&
         rd2.get_is_rna()) ) {
    ad1 = get_atom(rd1, atom::AT_O3p);
    ad2 = get_atom(rd2, atom::AT_P);
  }
  if(!ad1 || !ad2) {
    IMP_WARN("Residues incomplete: " << rd1 << " and " << rd2 <<std::endl);
    return;
  }
  Particle* p1 = ad1.get_particle();
  Particle* p2 = ad2.get_particle();

  Bonded b1,b2;
  if(Bonded::particle_is_instance(p1)) b1 = Bonded::decorate_particle(p1);
  else b1 = Bonded::setup_particle(p1);

  if(Bonded::particle_is_instance(p2)) b2 = Bonded::decorate_particle(p2);
  else b2 = Bonded::setup_particle(p2);

  IMP::atom::Bond bd = bond(b1, b2, IMP::atom::Bond::COVALENT);
}

void ForceFieldParameters::add_bonds(Residue rd) const {
  if (!rd) return;
  ResidueType type = rd.get_residue_type();
  if(residue_bonds_.find(type) == residue_bonds_.end()) return;

  const std::vector<Bond>& bonds = residue_bonds_.find(type)->second;
  for(unsigned int i=0; i<bonds.size(); i++) {
    Atom ad1 = get_atom(rd, bonds[i].type1_);
    Atom ad2 = get_atom(rd, bonds[i].type2_);
    if(!ad1 || !ad2) {
      IMP_WARN_ONCE("In residue " << rd << " could not find atom "
                    << bonds[i].type1_
                    << " or " << bonds[i].type2_ << std::endl, warn_context_);
      continue;
    }

    Particle* p1 = ad1.get_particle();
    Particle* p2 = ad2.get_particle();

    Bonded b1,b2;
    if(Bonded::particle_is_instance(p1)) b1 = Bonded::decorate_particle(p1);
    else b1 = Bonded::setup_particle(p1);

    if(Bonded::particle_is_instance(p2)) b2 = Bonded::decorate_particle(p2);
    else b2 = Bonded::setup_particle(p2);

    IMP::atom::Bond bd = bond(b1, b2, bonds[i].bond_type_);
  }
}

String ForceFieldParameters::get_force_field_atom_type(
                                      AtomType atom_type,
                                      ResidueType residue_type) const {
  static String empty_atom_type;
  if(atom_res_type_2_force_field_atom_type_.find(residue_type) ==
     atom_res_type_2_force_field_atom_type_.end()) {
    IMP_WARN_ONCE("Residue not found " << residue_type, warn_context_);
    return empty_atom_type;
  }

  //  string fixedAtomType = fixAtomType(atomName, residueName, NORM);
  const AtomTypeMap& atom_map =
    atom_res_type_2_force_field_atom_type_.find(residue_type)->second;
  if(atom_map.find(atom_type) == atom_map.end()) {
    IMP_WARN_ONCE("Atom not found " << atom_type
                  << " residue " << residue_type << std::endl, warn_context_);
    return empty_atom_type;
  }

  return atom_map.find(atom_type)->second.first;
}


ForceFieldParameters *default_force_field_parameters() {
  static IMP::internal::OwnerPointer<CharmmParameters> cfp
    (new CharmmParameters(get_data_path("top.lib"),
                          get_data_path("par.lib")));
  return cfp;
}

IMPATOM_END_NAMESPACE
