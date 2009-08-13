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
#include <IMP/atom/Mass.h>

#include <IMP/log.h>

#include <sstream>
#include <vector>
#include <limits>

IMPATOM_BEGIN_NAMESPACE

namespace {
  std::vector<Element> added_atom_names;

  Element element_from_name(const AtomType& at) {
    std::string atom_name = at.get_string();
    IMP_check(atom_name.length() > 0, "Invalid atom name.", ValueException);
    if (added_atom_names.size() > at.get_index()
        && added_atom_names[at.get_index()] == UNKNOWN_ELEMENT) {
      return added_atom_names[at.get_index()];
    }
    if (atom_name.find("HET_") != std::string::npos) {
      IMP_failure("You must call add_atom_name() to create a new atom name: "
                  << at,
                  InvalidStateException);
    } else {
      if (atom_name.find('H') != std::string::npos) {
        return H;
      } else if (atom_name.find('C') != std::string::npos) {
        return C;
      } else if (atom_name.find('O') != std::string::npos) {
        return O;
      } else if (atom_name.find('P') != std::string::npos) {
        return P;
      } else if (atom_name.find('N') != std::string::npos) {
        return N;
      } else if (atom_name.find('S') != std::string::npos) {
        return S;
      }
      IMP_failure("Could not figure out element for " << at,
                  InvalidStateException);
    }
  }
}

#define NAME_DEF(NAME) const AtomType AT_##NAME(AtomType::add_key(#NAME))
#define NAME_DEF2(NAME, STR) const AtomType AT_##NAME(AtomType::add_key(STR))
#define NAME_ALIAS(OLD_NAME, NAME) const AtomType AT_##NAME\
  (AtomType::add_alias(AT_##OLD_NAME, #NAME))

NAME_DEF(N);
NAME_DEF(H);
NAME_DEF(1H);
NAME_DEF(H1);
NAME_DEF(2H);
NAME_DEF(H2);
NAME_DEF(3H);
NAME_DEF(H3);
NAME_DEF(C);
NAME_DEF(O);
NAME_DEF(OXT);
NAME_ALIAS(OXT, OT1);
NAME_ALIAS(OXT, OT2);
NAME_DEF(CH3);

NAME_DEF(CA);
NAME_DEF(HA);
NAME_DEF(HA1);
NAME_DEF(HA2);

NAME_DEF(CB);
NAME_DEF(HB);
NAME_DEF(HB1);
NAME_DEF(HB2);
NAME_DEF(HB3);

NAME_DEF(CG);
NAME_DEF(CG1);
NAME_DEF(CG2);
NAME_DEF(HG);
NAME_DEF(HG1);
NAME_DEF(HG2);
NAME_DEF(HG3);
//NAME(HG1",Atom::AT_HG1},
NAME_DEF(1HG1);
NAME_DEF(2HG1);
NAME_DEF(3HG1);
NAME_DEF(1HG2);
NAME_DEF(2HG2);
NAME_DEF(3HG2);
NAME_DEF(OG);
NAME_DEF(OG1);
NAME_DEF(SG);

NAME_DEF(CD);
NAME_DEF(CD1);
NAME_DEF(CD2);
//NAME(HD1",Atom::AT_HD1},
//NAME(HD2",Atom::AT_HD2},
NAME_DEF(HD);
NAME_DEF(HD1);
NAME_DEF(HD2);
NAME_DEF(HD3);
NAME_DEF(1HD1);
NAME_DEF(2HD1);
NAME_DEF(3HD1);
NAME_DEF(1HD2);
NAME_DEF(2HD2);
NAME_DEF(3HD2);
NAME_DEF(SD);
NAME_DEF(OD1);
NAME_DEF(OD2);
NAME_DEF(ND1);
NAME_DEF(ND2);

NAME_DEF(CE);
NAME_DEF(CE1);
NAME_DEF(CE2);
NAME_DEF(CE3);
NAME_DEF(HE);
NAME_DEF(HE1);
NAME_DEF(HE2);
NAME_DEF(HE3);
//NAME(HE1",Atom::AT_HE1},
//NAME(HE2",Atom::AT_HE2},
//NAME(HE3",Atom::AT_HE3},
NAME_DEF(1HE2);
NAME_DEF(2HE2);
NAME_DEF(OE1);
NAME_DEF(OE2);
NAME_DEF(NE);
NAME_DEF(NE1);
NAME_DEF(NE2);

NAME_DEF(CZ);
NAME_DEF(CZ2);
NAME_DEF(CZ3);
NAME_DEF(NZ);
NAME_DEF(HZ);
NAME_DEF(HZ1);
NAME_DEF(HZ2);
NAME_DEF(HZ3);
//NAME(HZ1",Atom::AT_HZ2},
//NAME(HZ2",Atom::AT_HZ2},
//NAME(HZ3",Atom::AT_HZ3},

NAME_DEF(CH2);
NAME_DEF(NH1);
NAME_DEF(NH2);
NAME_DEF(OH);
NAME_DEF(HH);

NAME_DEF(1HH1);
NAME_DEF(2HH1);
NAME_DEF(HH2);
NAME_DEF(1HH2);
NAME_DEF(2HH2);
NAME_DEF(2HH3);
NAME_DEF(3HH3);
NAME_DEF(1HH3);
//NAME_ALIAS(1HH3, HH31);

//NAME(HH31);
//NAME(HH32);
//NAME(HH33);

NAME_DEF(P);
NAME_DEF(OP1);
NAME_DEF(OP2);
NAME_DEF2(O5p, "O5'");
NAME_DEF2(C5p,"C5'");
NAME_DEF2(H5pp, "H5''");
NAME_DEF2(C4p, "C4'");
NAME_DEF2(H4p, "H4'");
NAME_DEF2(H5p, "H5'");
NAME_DEF2(O4p, "O4'");
NAME_DEF2(C1p, "C1'");
NAME_DEF2(H1p, "H1'");
NAME_DEF2(C3p, "C3'");
NAME_DEF2(H3p, "H3'");
NAME_DEF2(O3p, "O3'");
NAME_DEF2(C2p, "C2'");
NAME_DEF2(H2p, "H2'");
NAME_DEF2(H2pp, "H2''");
NAME_DEF2(O2p, "O2'");
NAME_DEF2(HO2p, "HO2'");
NAME_DEF(N9);
NAME_DEF(C8);
NAME_DEF(H8);
NAME_DEF(N7);
NAME_DEF(C5);
NAME_DEF(C4);
NAME_DEF(N3);
NAME_DEF(C2);
NAME_DEF(N1);
NAME_DEF(C6);
NAME_DEF(N6);
NAME_DEF(H61);
NAME_DEF(H62);
NAME_DEF(O6);

NAME_DEF(N2);
NAME_DEF(H21);
NAME_DEF(H22);

NAME_DEF(H6);
NAME_DEF(H5);
NAME_DEF(O2);
NAME_DEF(N4);
NAME_DEF(H41);
NAME_DEF(H42);

NAME_DEF(O4);
NAME_DEF(C7);
NAME_DEF(H71);
NAME_DEF(H72);
NAME_DEF(H73);

// new
NAME_DEF(HH11);
NAME_DEF(HH12);
NAME_DEF(HH21);
NAME_DEF(HH22);
NAME_DEF(HD22);
NAME_DEF(HD11);
NAME_DEF(HD12);
NAME_DEF(HD21);
NAME_DEF(NO2);

NAME_DEF(UNKNOWN);

Atom Atom::setup_particle(Particle *p, AtomType t) {
  p->add_attribute(get_atom_type_key(), t.get_index());
  Hierarchy::setup_particle(p, Hierarchy::ATOM);
  p->add_attribute(get_element_key(), UNKNOWN_ELEMENT);
  Atom ret(p);
  Mass::setup_particle(p, 0);
  ret.set_atom_type(t);
  return ret;
}

Atom Atom::setup_particle(Particle *p, Atom o) {
  Atom ret=setup_particle(p, o.get_atom_type());
  if (o.get_charge() != 0) {
    ret.set_charge(o.get_charge());
  }
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
  Element e;
  if (t.get_index() < added_atom_names.size()
      && added_atom_names[t.get_index()] != UNKNOWN_ELEMENT) {
    e= added_atom_names[t.get_index()];
  } else {
    e=element_from_name(t);
  }
  IMP_assert(e != UNKNOWN_ELEMENT,
             "Internal error setting element name from " << t);
  get_particle()->set_value(get_element_key(), e);
  Mass(get_particle()).set_mass(get_element_table().get_mass(e));
}

IntKey Atom::get_atom_type_key() {
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


AtomType add_atom_type(std::string name, Element e) {
  IMP_check(!AtomType::get_key_exists(name),
            "An AtomType with that name already exists: "
            << name, ValueException);
  IMP_check(e != UNKNOWN_ELEMENT, "Atom type must have element: " << name,
            ValueException);
  AtomType ret= AtomType::add_key(name.c_str());
  added_atom_names.resize(std::max(added_atom_names.size(),
                                   static_cast<std::size_t>(ret.get_index()+1)),
                          UNKNOWN_ELEMENT);
  added_atom_names[ret.get_index()]=e;
  return ret;
}


bool atom_type_exists(std::string name) {
  return AtomType::get_key_exists(name);
}

IMPATOM_END_NAMESPACE
