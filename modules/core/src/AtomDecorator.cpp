/**
 *  \file AtomDecorator.cpp   \brief Simple atoms decorator.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/AtomDecorator.h>
#include <IMP/core/MolecularHierarchyDecorator.h>

#include <IMP/log.h>

#include <sstream>
#include <vector>
#include <limits>

IMPCORE_BEGIN_NAMESPACE

#define TYPE_DEF(STR) const AtomType AT_##STR(#STR);
#define TYPE_DEF2(NAME, STR) const AtomType AT_##NAME(#STR);

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
//TYPE(HG1",AtomDecorator::AT_HG1},
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
//TYPE(HD1",AtomDecorator::AT_HD1},
//TYPE(HD2",AtomDecorator::AT_HD2},
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
//TYPE(HE1",AtomDecorator::AT_HE1},
//TYPE(HE2",AtomDecorator::AT_HE2},
//TYPE(HE3",AtomDecorator::AT_HE3},
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
//TYPE(HZ1",AtomDecorator::AT_HZ2},
//TYPE(HZ2",AtomDecorator::AT_HZ2},
//TYPE(HZ3",AtomDecorator::AT_HZ3},

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

//TYPE(HH31);
//TYPE(HH32);
//TYPE(HH33);

TYPE_DEF(P);
TYPE_DEF(OP1);
TYPE_DEF(OP2);
TYPE_DEF2(O5p, O5*);
TYPE_DEF2(C5p,C5*);
TYPE_DEF2(H5pp, H5**);
TYPE_DEF2(C4p, C4*);
TYPE_DEF2(H4p, H4*);
TYPE_DEF2(H5p, H5*);
TYPE_DEF2(O4p, O4*);
TYPE_DEF2(C1p, C1*);
TYPE_DEF2(H1p, H1*);
TYPE_DEF2(C3p, C3*);
TYPE_DEF2(H3p, H3*);
TYPE_DEF2(O3p, O3*);
TYPE_DEF2(C2p, C2*);
TYPE_DEF2(H2p, H2*);
TYPE_DEF2(H2pp, H2**);
TYPE_DEF2(O2p, O2*);
TYPE_DEF2(HO2p, HO2*);
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

AtomType atom_type_from_pdb_string(std::string nm) {
  if (!AtomType::get_key_exists(nm)) {
    std::ostringstream oss;
    oss<< "AtomType " << nm << " does not exist.";
    throw ValueException(oss.str().c_str());
  }
  return AtomType(nm.c_str());
}

void AtomDecorator::show(std::ostream &out, std::string prefix) const
{
  //out <<prefix << "Element:"<< get_element() << std::endl;
  out << prefix<< "Type: "<< get_type();
  if (get_input_index() != -1) {
    out << " atom number: " << get_input_index();
  }
  out << std::endl;
  XYZDecorator::show(out, prefix);
}


void AtomDecorator::set_type(AtomType t)
{
  // ultimate the secondary info should be set from a
  // better source. But this is good enough for now.
  get_particle()->set_value(get_type_key(), t.get_index());
}

IntKey AtomDecorator::get_type_key() {
  static IntKey k("atom_type");
  return k;
}

IntKey AtomDecorator::get_element_key() {
  static IntKey k("element");
  return k;
}

FloatKey AtomDecorator::get_mass_key() {
  static FloatKey k("atom_mass");
  return k;
}

FloatKey AtomDecorator::get_charge_key() {
  static FloatKey k("atom_charge");
  return k;
}

IntKey AtomDecorator::get_input_index_key() {
  static IntKey k("pdb_atom_index");
  return k;
}

unsigned int get_residue_index(AtomDecorator d) {
  return get_residue(d).get_index();
}


ResidueType get_residue_type(AtomDecorator d) {
  return get_residue(d).get_type();
}

ResidueDecorator get_residue(AtomDecorator d) {
  MolecularHierarchyDecorator mhd(d.get_particle());
  do {
    mhd= mhd.get_parent();
    if (mhd== MolecularHierarchyDecorator()) {
      throw InvalidStateException("Atom is not the child of a residue");
    }
  } while (!ResidueDecorator::is_instance_of(mhd.get_particle()));
  ResidueDecorator rd(mhd.get_particle());
  return rd;
}

AtomDecorator get_atom(ResidueDecorator rd, AtomType at) {
  MolecularHierarchyDecorator mhd(rd.get_particle());
  for (unsigned int i=0; i< mhd.get_number_of_children(); ++i) {
    AtomDecorator a(mhd.get_child(i).get_particle());
    if (a.get_type() == at) return a;
  }
  throw InvalidStateException("Atom not found");
}

std::string AtomDecorator::get_pdb_string(int index) {
  std::stringstream out;
  Particle *p = get_particle();
  out.setf(std::ios::left, std::ios::adjustfield);
  out.width(6);
  if (get_residue_type(*this) == UNK) {
    out<<"HETATM";
  }
  else {
    out << "ATOM";
  }
  //7-11 : atom id
  out.setf(std::ios::right, std::ios::adjustfield);
  out.width(5);
  if (index==-1) {
    out << get_input_index_key();
  }
  else {
    out << index;
  }
  // 12: skip an undefined position
  out.width(1);
  out<< " ";

  // 13-16: atom type
  out.setf(std::ios::left, std::ios::adjustfield);
  out.width(1);
  std::string atom_type = get_type().get_string();
  if (atom_type.size()<4) {
    out << " ";
    out.width(3);
    out<<atom_type;
  }
  else {
    out<<atom_type;
  }
  // 17: skip the alternate indication position
  out.width(1);
  out << " ";

  // 18-20 : residue name
  out.width(3);
  out<< get_residue_type(*this).get_string();
  //skip 21
  out.width(1);
  out<<" ";
  // 22: chain identifier
  out << " ";
  //23-26: residue number
  out.setf(std::ios::right, std::ios::adjustfield);
  out.width(4);
  out << get_residue_index(*this);
  //27: residue insertion code
  out.width(1);
  out << " "; //TODO
  out.setf(std::ios::fixed, std::ios::floatfield);
  out << "   "; // skip 3 undefined positions (28-30)
  XYZDecorator xyz= XYZDecorator::cast(p);
  // coordinates (31-38,39-46,47-54)
  out.width(8);
  out.precision(3);
  out << xyz.get_x();
  out.width(8);
  out.precision(3);
  out << xyz.get_y();
  out.width(8);
  out.precision(3);
  out << xyz.get_z();

  //55:60 occupancy
  out.width(6);
  out.precision(2);
  out << ""; //TODO
  //61-66: temp. factor
  out.width(6);
  out.precision(2);
  out << ""<<std::endl; //TODO
  return out.str();
}

IMPCORE_END_NAMESPACE
