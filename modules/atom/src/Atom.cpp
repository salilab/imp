/**
 *  \file Atom.cpp   \brief Simple atoms decorator.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/atom/Atom.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/Chain.h>
#include <IMP/atom/element.h>
#include <IMP/atom/pdb.h>
#include <IMP/core/XYZ.h>

#include <IMP/log.h>

#include <sstream>
#include <vector>
#include <limits>

IMPATOM_BEGIN_NAMESPACE

#define TYPE_DEF(NAME) const AtomType AT_##NAME(#NAME)
#define TYPE_DEF2(NAME, STR) const AtomType AT_##NAME(STR)
#define TYPE_ALIAS(OLD_NAME, NAME) const AtomType AT_##NAME\
  (AtomType::add_alias(AT_##OLD_NAME, #NAME))

TYPE_DEF(N);
TYPE_DEF(H);
TYPE_DEF(1H);
TYPE_DEF(H1);
TYPE_DEF(2H);
TYPE_DEF(H2);
TYPE_DEF(3H);
TYPE_DEF(H3);
TYPE_DEF(C);
TYPE_DEF(O);
TYPE_DEF(OXT);
TYPE_ALIAS(OXT, OT1);
TYPE_ALIAS(OXT, OT2);
TYPE_DEF(CH3);

TYPE_DEF(CA);
TYPE_DEF(HA);
TYPE_DEF(1HA);
TYPE_DEF(2HA);

TYPE_DEF(CB);
TYPE_DEF(HB);
TYPE_DEF(1HB);
TYPE_DEF(2HB);
TYPE_DEF(3HB);

TYPE_DEF(CG);
TYPE_DEF(CG1);
TYPE_DEF(CG2);
TYPE_DEF(HG);
TYPE_DEF(1HG);
TYPE_DEF(2HG);
//TYPE(HG1",Atom::AT_HG1},
TYPE_DEF(1HG1);
TYPE_DEF(2HG1);
TYPE_DEF(3HG1);
TYPE_DEF(1HG2);
TYPE_DEF(2HG2);
TYPE_DEF(3HG2);
TYPE_DEF(OG);
TYPE_DEF(OG1);
TYPE_DEF(SG);

TYPE_DEF(CD);
TYPE_DEF(CD1);
TYPE_DEF(CD2);
//TYPE(HD1",Atom::AT_HD1},
//TYPE(HD2",Atom::AT_HD2},
TYPE_DEF(HD);
TYPE_DEF(1HD);
TYPE_DEF(2HD);
TYPE_DEF(3HD);
TYPE_DEF(1HD1);
TYPE_DEF(2HD1);
TYPE_DEF(3HD1);
TYPE_DEF(1HD2);
TYPE_DEF(2HD2);
TYPE_DEF(3HD2);
TYPE_DEF(SD);
TYPE_DEF(OD1);
TYPE_DEF(OD2);
TYPE_DEF(ND1);
TYPE_DEF(ND2);

TYPE_DEF(CE);
TYPE_DEF(CE1);
TYPE_DEF(CE2);
TYPE_DEF(CE3);
TYPE_DEF(HE);
TYPE_DEF(1HE);
TYPE_DEF(2HE);
TYPE_DEF(3HE);
//TYPE(HE1",Atom::AT_HE1},
//TYPE(HE2",Atom::AT_HE2},
//TYPE(HE3",Atom::AT_HE3},
TYPE_DEF(1HE2);
TYPE_DEF(2HE2);
TYPE_DEF(OE1);
TYPE_DEF(OE2);
TYPE_DEF(NE);
TYPE_DEF(NE1);
TYPE_DEF(NE2);

TYPE_DEF(CZ);
TYPE_DEF(CZ2);
TYPE_DEF(CZ3);
TYPE_DEF(NZ);
TYPE_DEF(HZ);
TYPE_DEF(1HZ);
TYPE_DEF(2HZ);
TYPE_DEF(3HZ);
//TYPE(HZ1",Atom::AT_HZ2},
//TYPE(HZ2",Atom::AT_HZ2},
//TYPE(HZ3",Atom::AT_HZ3},

TYPE_DEF(CH2);
TYPE_DEF(NH1);
TYPE_DEF(NH2);
TYPE_DEF(OH);
TYPE_DEF(HH);

TYPE_DEF(1HH1);
TYPE_DEF(2HH1);
TYPE_DEF(HH2);
TYPE_DEF(1HH2);
TYPE_DEF(2HH2);
TYPE_DEF(2HH3);
TYPE_DEF(3HH3);
TYPE_DEF(1HH3);
//TYPE_ALIAS(1HH3, HH31);

//TYPE(HH31);
//TYPE(HH32);
//TYPE(HH33);

TYPE_DEF(P);
TYPE_DEF(OP1);
TYPE_DEF(OP2);
TYPE_DEF2(O5p, "O5'");
TYPE_DEF2(C5p,"C5'");
TYPE_DEF2(H5pp, "H5''");
TYPE_DEF2(C4p, "C4'");
TYPE_DEF2(H4p, "H4'");
TYPE_DEF2(H5p, "H5'");
TYPE_DEF2(O4p, "O4'");
TYPE_DEF2(C1p, "C1'");
TYPE_DEF2(H1p, "H1'");
TYPE_DEF2(C3p, "C3'");
TYPE_DEF2(H3p, "H3'");
TYPE_DEF2(O3p, "O3'");
TYPE_DEF2(C2p, "C2'");
TYPE_DEF2(H2p, "H2'");
TYPE_DEF2(H2pp, "H2''");
TYPE_DEF2(O2p, "O2'");
TYPE_DEF2(HO2p, "HO2'");
TYPE_DEF(N9);
TYPE_DEF(C8);
TYPE_DEF(H8);
TYPE_DEF(N7);
TYPE_DEF(C5);
TYPE_DEF(C4);
TYPE_DEF(N3);
TYPE_DEF(C2);
TYPE_DEF(N1);
TYPE_DEF(C6);
TYPE_DEF(N6);
TYPE_DEF(H61);
TYPE_DEF(H62);
TYPE_DEF(O6);

TYPE_DEF(N2);
TYPE_DEF(H21);
TYPE_DEF(H22);

TYPE_DEF(H6);
TYPE_DEF(H5);
TYPE_DEF(O2);
TYPE_DEF(N4);
TYPE_DEF(H41);
TYPE_DEF(H42);

TYPE_DEF(O4);
TYPE_DEF(C7);
TYPE_DEF(H71);
TYPE_DEF(H72);
TYPE_DEF(H73);

TYPE_DEF(UNKNOWN);

Atom Atom::setup_particle(Particle *p, AtomType t) {
  p->add_attribute(get_type_key(), t.get_index());
  Hierarchy::setup_particle(p, Hierarchy::ATOM);
  Atom ret(p);
  ret.set_atom_type(t);
  return ret;
}

Atom Atom::setup_particle(Particle *p, Atom o) {
  p->add_attribute(get_type_key(), o.get_atom_type().get_index());
  Hierarchy::setup_particle(p, Hierarchy::ATOM);
  Atom ret(p);
  if (o.get_element() != UNKNOWN_ELEMENT) {
    ret.set_element(o.get_element());
  }
  if (o.get_mass() > 0) {
    ret.set_mass(o.get_mass());
  }
  if (o.get_charge() != 0) {
    ret.set_charge(o.get_charge());
  }
  if (o.get_input_index() != -1) {
    ret.set_input_index(o.get_input_index());
  }
  return ret;
}

AtomType atom_type_from_pdb_string(std::string nm) {
  if (!AtomType::get_key_exists(nm)) {
    std::ostringstream oss;
    oss<< "AtomType " << nm << " does not exist.";
    throw ValueException(oss.str().c_str());
  }
  return AtomType(nm.c_str());
}

void Atom::show(std::ostream &out, std::string prefix) const
{
  out << prefix << "Element:"<< get_element()
      << " mass " << get_mass() << std::endl;
  out << prefix << "Type: "<< get_atom_type();
  if (get_input_index() != -1) {
    out << " atom number: " << get_input_index();
  }
  out << std::endl;
}


void Atom::set_atom_type(AtomType t)
{
  get_particle()->set_value(get_type_key(), t.get_index());
}

IntKey Atom::get_type_key() {
  static IntKey k("atom_type");
  return k;
}

IntKey Atom::get_element_key() {
  static IntKey k("element");
  return k;
}

FloatKey Atom::get_charge_key() {
  static FloatKey k("atom_charge");
  return k;
}

IntKey Atom::get_input_index_key() {
  static IntKey k("pdb_atom_index");
  return k;
}

int get_residue_index(Atom d) {
  return get_residue(d).get_index();
}


ResidueType get_residue_type(Atom d) {
  return get_residue(d).get_residue_type();
}

Residue get_residue(Atom d) {
  Hierarchy mhd(d.get_particle());
  do {
    mhd= mhd.get_parent();
    if (mhd== Hierarchy()) {
      throw InvalidStateException("Atom is not the child of a residue");
    }
  } while (!Residue::particle_is_instance(mhd.get_particle()));
  Residue rd(mhd.get_particle());
  return rd;
}

Atom get_atom(Residue rd, AtomType at) {
  Hierarchy mhd(rd.get_particle());
  for (unsigned int i=0; i< mhd.get_number_of_children(); ++i) {
    Atom a(mhd.get_child(i).get_particle());
    if (a.get_atom_type() == at) return a;
  }
  IMP_LOG(VERBOSE, "Atom not found " << at << std::endl);
  return Atom();
}

char get_chain(Atom d) {
  Residue rd = get_residue(d);
  return get_chain(rd);
}


std::string Atom::get_pdb_string(int index) {
  algebra::Vector3D v =
    core::XYZ::decorate_particle(get_particle()).get_coordinates();
  int atom_index = index;
  if (index==-1) atom_index = get_input_index();
  return atom::get_pdb_string(v, atom_index, get_atom_type(),
                              get_residue_type(*this), get_chain(*this),
                              get_residue_index(*this),
                              get_residue(*this).get_insertion_code(),
                              (Element)get_element());
}


AtomType add_atom_type(std::string name) {
  IMP_check(!AtomType::get_key_exists(name),
            "An AtomType with that name already exists: "
            << name, ValueException);
  AtomType ret(name.c_str());
  return ret;
}

IMPATOM_END_NAMESPACE
