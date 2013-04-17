/**
 *  \file Atom.cpp   \brief Simple atoms decorator.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/Atom.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/Chain.h>
#include <IMP/atom/element.h>
#include <IMP/atom/pdb.h>
#include <IMP/core/XYZ.h>
#include <IMP/atom/Mass.h>

#include <IMP/log.h>

#include <sstream>
#include <vector>
#include <limits>

IMPATOM_BEGIN_NAMESPACE

namespace {
  base::Vector<Element> added_atom_names;
}

#define NAME_DEF(NAME, ELEMENT) const AtomType AT_##NAME\
  = add_atom_type(#NAME, ELEMENT)
#define NAME_DEF2(NAME, STR, ELEMENT) const AtomType AT_##NAME\
  = add_atom_type(STR, ELEMENT)
#define NAME_ALIAS(OLD_NAME, NAME) const AtomType AT_##NAME\
  (AtomType::add_alias(AT_##OLD_NAME, #NAME))

NAME_DEF(N, N);
NAME_DEF(H, H);
NAME_DEF(1H, H);
NAME_DEF(H1, H);
NAME_DEF(2H, H);
NAME_DEF(H2, H);
NAME_DEF(3H, H);
NAME_DEF(H3, H);
NAME_DEF(C, C);
NAME_DEF(O, O);
NAME_DEF(OXT, O);
NAME_DEF(OT1, O);
NAME_DEF(OT2, O);
NAME_DEF(CH3, C);
NAME_DEF(CH, C);

NAME_DEF(CA, C);
NAME_DEF(HA, H);
NAME_DEF(HA1, H);
NAME_DEF(HA2, H);
NAME_DEF(HA3, H);

NAME_DEF(CB, C);
NAME_DEF(HB, H);
NAME_DEF(HB1, H);
NAME_DEF(HB2, H);
NAME_DEF(HB3, H);

NAME_DEF(CG, C);
NAME_DEF(CG1, C);
NAME_DEF(CG2, C);
NAME_DEF(HG, H);
NAME_DEF(HG1, H);
NAME_DEF(HG2, H);
NAME_DEF(HG3, H);
NAME_DEF(HG11, H);
NAME_DEF(HG21, H);
NAME_DEF(HG31, H);
NAME_DEF(HG12, H);
NAME_DEF(HG13, H);
NAME_DEF(HG22, H);
NAME_DEF(HG23, H);
NAME_DEF(HG32, H);
NAME_DEF(OG, O);
NAME_DEF(OG1, O);
NAME_DEF(SG, S);

NAME_DEF(CD, C);
NAME_DEF(CD1, C);
NAME_DEF(CD2, C);
NAME_DEF(HD, H);
NAME_DEF(HD1, H);
NAME_DEF(HD2, H);
NAME_DEF(HD3, H);
NAME_DEF(HD11, H);
NAME_DEF(HD21, H);
NAME_DEF(HD31, H);
NAME_DEF(HD12, H);
NAME_DEF(HD13, H);
NAME_DEF(HD22, H);
NAME_DEF(HD23, H);
NAME_DEF(HD32, H);
NAME_DEF(SD, S);
NAME_DEF(OD1, O);
NAME_DEF(OD2, O);
NAME_DEF(ND1, N);
NAME_DEF(ND2, N);

NAME_DEF(CE, C);
NAME_DEF(CE1, C);
NAME_DEF(CE2, C);
NAME_DEF(CE3, C);
NAME_DEF(HE, H);
NAME_DEF(HE1, H);
NAME_DEF(HE2, H);
NAME_DEF(HE3, H);
NAME_DEF(HE21, H);
NAME_DEF(HE22, H);
NAME_DEF(OE1, O);
NAME_DEF(OE2, O);
NAME_DEF(NE, N);
NAME_DEF(NE1, N);
NAME_DEF(NE2, N);

NAME_DEF(CZ, C);
NAME_DEF(CZ2, C);
NAME_DEF(CZ3, C);
NAME_DEF(NZ, N);
NAME_DEF(HZ, H);
NAME_DEF(HZ1, H);
NAME_DEF(HZ2, H);
NAME_DEF(HZ3, H);

NAME_DEF(CH2, C);
NAME_DEF(NH1, N);
NAME_DEF(NH2, N);
NAME_DEF(OH, O);
NAME_DEF(HH, H);

NAME_DEF(HH11, H);
NAME_DEF(HH21, H);
NAME_DEF(HH2, H);
NAME_DEF(HH12, H);
NAME_DEF(HH22, H);
NAME_DEF(HH23, H);
NAME_DEF(HH33, H);
NAME_DEF(HH13, H);
//NAME_ALIAS(1HH3, HH31);
NAME_DEF(P, P);
NAME_DEF(OP1, O);
NAME_DEF(OP2, O);
NAME_DEF(OP3, O);
NAME_DEF2(O5p, "O5'", O);
NAME_DEF2(C5p,"C5'", C);
NAME_DEF2(H5pp, "H5''", H);
NAME_DEF2(C4p, "C4'", C);
NAME_DEF2(H4p, "H4'", H);
NAME_DEF2(H5p, "H5'", H);
NAME_DEF2(O4p, "O4'", O);
NAME_DEF2(C1p, "C1'", C);
NAME_DEF2(H1p, "H1'", H);
NAME_DEF2(C3p, "C3'", C);
NAME_DEF2(H3p, "H3'", H);
NAME_DEF2(O3p, "O3'", O);
NAME_DEF2(C2p, "C2'", C);
NAME_DEF2(H2p, "H2'", H);
NAME_DEF2(H2pp, "H2''", H);
NAME_DEF2(O2p, "O2'", O);
NAME_DEF2(HO2p, "HO2'", H);
NAME_DEF(N9, N);
NAME_DEF(C8, C);
NAME_DEF(H8, H);
NAME_DEF(N7, N);
NAME_DEF(C5, C);
NAME_DEF(C4, C);
NAME_DEF(N3, N);
NAME_DEF(C2, C);
NAME_DEF(N1, N);
NAME_DEF(C6, C);
NAME_DEF(N6, N);
NAME_DEF(H61, H);
NAME_DEF(H62, H);
NAME_DEF(O6, O);

NAME_DEF(N2, N);
NAME_DEF(NT, N);
NAME_DEF(H21, H);
NAME_DEF(H22, H);

NAME_DEF(H6, H);
NAME_DEF(H5, H);
NAME_DEF(O2, O);
NAME_DEF(N4, N);
NAME_DEF(H41, H);
NAME_DEF(H42, H);

NAME_DEF(O4, O);
NAME_DEF(C7, C);
NAME_DEF(H71, H);
NAME_DEF(H72, H);
NAME_DEF(H73, H);

NAME_DEF(O1A, O);
NAME_DEF(O2A, O);
NAME_DEF(O3A, O);
NAME_DEF(O1B, O);
NAME_DEF(O2B, O);
NAME_DEF(O3B, O);

// new
NAME_DEF(NO2, N);

NAME_DEF(UNKNOWN, UNKNOWN_ELEMENT);

Atom Atom::setup_particle(Model *m,
                          ParticleIndex pi, AtomType t) {
  Particle *p= m->get_particle(pi);
  p->add_attribute(get_atom_type_key(), t.get_index());
  if (!Hierarchy::particle_is_instance(p)) {
    Hierarchy::setup_particle(p);
  }
  p->add_attribute(get_element_key(), UNKNOWN_ELEMENT);
  //p->add_attribute(get_occupancy_key(), 1.00);
  //p->add_attribute(get_tempFactor_key(), 0.00);

  Atom ret(p);
  if (!Mass::particle_is_instance(p)) {
    Mass::setup_particle(p, 0);
  }
  ret.set_atom_type(t);
  return ret;
}

Atom Atom::setup_particle(Particle *p, Atom o) {
  Atom ret=setup_particle(p, o.get_atom_type());
  return ret;
}

void Atom::show(std::ostream &out) const
{
  out << "  element:"<< get_element_table().get_name(get_element());
  out << " type: "<< get_atom_type();
  if (get_input_index() != -1) {
    out << " input index: " << get_input_index();
  }
  if (core::XYZ::particle_is_instance(get_particle())) {
    out << " coords: " << core::XYZ(get_particle());
  }
}


void Atom::set_atom_type(AtomType t)
{
  get_particle()->set_value(get_atom_type_key(), t.get_index());
  Element e= get_element_for_atom_type(t);
  if (e != UNKNOWN_ELEMENT) {
    set_element(e);
  }
}

IntKey Atom::get_atom_type_key() {
  static IntKey k("atom_type");
  return k;
}

IntKey Atom::get_element_key() {
  static IntKey k("element");
  return k;
}

IntKey Atom::get_input_index_key() {
  static IntKey k("pdb_atom_index");
  return k;
}

FloatKey Atom::get_occupancy_key() {
  static FloatKey k("occupancy");
  return k;
}

FloatKey Atom::get_temperature_factor_key() {
  static FloatKey k("tempFactor");
  return k;
}

Residue get_residue(Atom d, bool nothrow) {
  Hierarchy mhd(d.get_particle());
  do {
    mhd= mhd.get_parent();
    if (mhd== Hierarchy()) {
      if (nothrow) return Residue();
      else {
        IMP_THROW("Atom is not the child of a residue "  << d,
                  ValueException);
      }
    }
  } while (!Residue::particle_is_instance(mhd.get_particle()));
  Residue rd(mhd.get_particle());
  return rd;
}

Atom get_atom(Residue rd, AtomType at) {
  Hierarchy mhd(rd.get_particle());
  for (unsigned int i=0; i< mhd.get_number_of_children(); ++i) {
    Atom a(mhd.get_child(i));
    if (a.get_atom_type() == at) return a;
  }
  IMP_LOG_VERBOSE( "Atom not found " << at << std::endl);
  return Atom();
}

void Atom::set_element(Element e) {
  get_particle()->set_value(get_element_key(), e);
  Mass(get_particle()).set_mass(get_element_table().get_mass(e));
}


AtomType add_atom_type(std::string name, Element e) {
  IMP_USAGE_CHECK(!AtomType::get_key_exists(name),
            "An AtomType with that name already exists: "
            << name);
  AtomType ret(AtomType::add_key(name));
  added_atom_names.resize(std::max(added_atom_names.size(),
                                   static_cast<std::size_t>(ret.get_index()+1)),
                          UNKNOWN_ELEMENT);
  added_atom_names[ret.get_index()]=e;
  return ret;
}

Element get_element_for_atom_type(AtomType at) {
  IMP_USAGE_CHECK(at.get_index() < added_atom_names.size(),
                  "Invalid AtomType index " << at);
  return added_atom_names[at.get_index()];
}

bool get_atom_type_exists(std::string name) {
  return AtomType::get_key_exists(name);
}

IMPATOM_END_NAMESPACE
