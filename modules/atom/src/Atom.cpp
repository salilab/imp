/**
 *  \file Atom.cpp   \brief Simple atoms decorator.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/atom/Atom.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/Chain.h>
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

namespace {
  struct ElementData {
    AtomType at;
    Atom::Element e;
  };

  ElementData element_data[]=
    {{AT_N, Atom::N},
     {AT_H, Atom::H},
     {AT_1H, Atom::H},
     {AT_2H, Atom::H},
     {AT_3H, Atom::H},
     {AT_C, Atom::C},
     {AT_O, Atom::O},
     {AT_OXT, Atom::O},
     {AT_CH3, Atom::C},

     {AT_CA, Atom::C},
     {AT_HA, Atom::H},
     {AT_1HA, Atom::H},
     {AT_2HA, Atom::H},

     {AT_CB, Atom::C},
     {AT_HB, Atom::H},
     {AT_1HB, Atom::H},
     { AT_2HB, Atom::H},
     { AT_3HB, Atom::H},

     {AT_CG, Atom::C},
     {AT_CG1, Atom::C},
     {AT_CG2, Atom::C},
     {AT_HG, Atom::H},
     { AT_1HG, Atom::H},
     { AT_2HG, Atom::H},
     //{AT_HG1},
     {AT_1HG1, Atom::H},
     {AT_2HG1, Atom::H},
     {AT_3HG1, Atom::H},
     {AT_1HG2, Atom::H},
     {AT_2HG2, Atom::H},
     {AT_3HG2, Atom::H},
     {AT_OG, Atom::O},
     {AT_OG1, Atom::O},
     {AT_SG, Atom::S},

     {AT_CD, Atom::C},
     {AT_CD1, Atom::C},
     {AT_CD2, Atom::C},
     //{AT_HD1},
     //{AT_HD2},
     {AT_HD, Atom::H},
     {AT_1HD, Atom::H},
     {AT_2HD, Atom::H},
     {AT_3HD, Atom::H},
     {AT_1HD1, Atom::H},
     {AT_2HD1, Atom::H},
     {AT_3HD1, Atom::H},
     {AT_1HD2, Atom::H},
     {AT_2HD2, Atom::H},
     {AT_3HD2, Atom::H},
     {AT_SD, Atom::S},
     {AT_OD1, Atom::O},
     {AT_OD2, Atom::O},
     {AT_ND1, Atom::N},
     {AT_ND2, Atom::N},

     {AT_CE, Atom::C},
     {AT_CE1, Atom::C},
     {AT_CE2, Atom::C},
     {AT_CE3, Atom::C},
     {AT_HE, Atom::H},
     {AT_1HE, Atom::H},{AT_1HE, Atom::H},
     {AT_2HE, Atom::H},{AT_2HE, Atom::H},
     {AT_3HE, Atom::H},{AT_3HE, Atom::H},
     //{AT_HE1},
     //{AT_HE2},
     //{AT_HE3},
     {AT_1HE2, Atom::H},{AT_1HE2, Atom::H},
     {AT_2HE2, Atom::H},{AT_2HE2, Atom::H},
     {AT_OE1, Atom::O},
     {AT_OE2, Atom::O},
     {AT_NE, Atom::N},
     {AT_NE1, Atom::N},
     {AT_NE2, Atom::N},

     {AT_CZ, Atom::C},
     {AT_CZ2, Atom::C},
     {AT_CZ3, Atom::C},
     {AT_NZ, Atom::N},
     {AT_HZ, Atom::H},
     {AT_1HZ, Atom::H},
     {AT_2HZ, Atom::H},
     {AT_3HZ, Atom::H},
     //{AT_HZ2},
     //{AT_HZ2},
     //{AT_HZ3},

     {AT_CH2, Atom::C},
     {AT_NH1, Atom::N},
     {AT_NH2, Atom::N},
     {AT_OH, Atom::O},
     {AT_HH, Atom::H},

     { AT_1HH1, Atom::H},
     { AT_2HH1, Atom::H},
     {AT_HH2, Atom::H},
     { AT_1HH2, Atom::H},
     { AT_2HH2, Atom::H},
     { AT_1HH2, Atom::H},

     { AT_1HH3, Atom::H},
     { AT_2HH3, Atom::H},
     { AT_3HH3, Atom::H},

     { AT_P, Atom::P},
     { AT_OP1, Atom::O},
     { AT_OP2, Atom::O},
     { AT_O5p, Atom::O},
     { AT_H5p, Atom::C},
     { AT_H5pp, Atom::H},
     { AT_C4p, Atom::C},
     { AT_H4p, Atom::H},
     { AT_O4p, Atom::O},
     { AT_C1p, Atom::C},
     { AT_H1p, Atom::H},
     { AT_C3p, Atom::C},
     { AT_H3p, Atom::H},
     { AT_O3p, Atom::O},
     { AT_C2p, Atom::C},
     { AT_H2p, Atom::H},
     { AT_H2pp, Atom::H},
     { AT_O2p, Atom::O},
     { AT_HO2p, Atom::O},
     { AT_N9, Atom::N},
     { AT_C8, Atom::C},
     { AT_H8, Atom::H},
     { AT_N7, Atom::N},
     { AT_C5, Atom::C},
     { AT_C4, Atom::C},
     { AT_N3, Atom::N},
     { AT_C2, Atom::C},
     { AT_H2, Atom::H},
     { AT_N1, Atom::N},
     { AT_C6, Atom::C},
     { AT_N6, Atom::N},
     { AT_H61, Atom::H},
     { AT_H62, Atom::H},
     { AT_O6, Atom::O},
     { AT_H1, Atom::H},
     { AT_N2, Atom::N},
     { AT_H21, Atom::H},
     { AT_H22, Atom::H},

     { AT_H6, Atom::H},
     { AT_H5, Atom::H},
     { AT_O2, Atom::O},
     { AT_N4, Atom::N},
     { AT_H41, Atom::H},
     { AT_H42,Atom::H},
     { AT_H3, Atom::H},
     { AT_O4, Atom::O},
     { AT_C7, Atom::C},
     { AT_H71, Atom::H},
     { AT_H72,Atom::H},
     { AT_H73,Atom::H},
     { AT_UNKNOWN, Atom::UNKNOWN_ELEMENT}};

  // from http://physics.nist.gov/cgi-bin/Compositions/stand_alone.pl
  float mass[] = { 0.0, 1.007, 4.002, 6.941, 9.012, 10.811, 12.0107, 14.0067,
                   //UNKNOWN_ELEMENT=0, H=1, He=2, Li=3, Be=4, B=5, C=6, N=7,
                   15.9994, 18.998, 20.1797, 22.989, 24.3050, 26.981, 28.0855,
                   //O=8, F=9, Ne=10, Na=11, Mg=12, Al=13, Si=14,
                   30.973, 32.065, 35.453, 39.948, 39.0983, 40.078, 44.955,
                   //P=15, S=16, Cl=17, Ar=18, K=19, Ca=20, Sc=21,
                   47.867, 50.9415, 51.9961, 54.938, 55.845, 58.933, 58.6934,
                   //Ti=22, V=23, Cr=24, Mn=25, Fe=26, Co=27, Ni=28,
                   63.546, 65.409, 69.723, 72.64, 74.921, 78.96, 79.904,
                   //Cu=29, Zn=30, Ga=31, Ge=32, As=33, Se=34, Br=35,
                   83.798, 85.4678, 87.62, 88.905, 91.224, 92.906, 95.94,
                   //Kr=36, Rb=37, Sr=38, Y=39, Zr=40, Nb=41, Mo=42,
                   97.907, 101.07, 102.905, 106.42, 107.8682, 112.411, 114.818,
                   //Tc=43, Ru=44, Rh=45, Pd=46, Ag=47, Cd=48, In=49,
                   118.71, 121.76, 127.60, 126.90, 131.29, 132.91, 137.33,
                   //Sn=50, Sb=51, Te=52, I=53, Xe=54, Cs=55, Ba=56,
                   138.91, 140.12, 140.91, 144.24, 145, 150.36, 151.96,
                   //La=57, Ce=58, Pr=59, Nd=60, Pm=61, Sm=62, Eu=63,
                   157.25, 158.93, 162.50, 164.93, 167.26, 168.93, 173.04,
                   //Gd=64, Tb=65, Dy=66, Ho=67, Er=68, Tm=69, Yb=70,
                   174.97, 178.49, 180.95, 183.84, 186.21, 190.23, 192.22,
                   //Lu=71, Hf=72, Ta=73, W=74, Re=75, Os=76, Ir=77,
                   195.08, 196.97, 200.59, 204.38, 207.2, 208.98, 208.98,
                   //Pt=78, Au=79, Hg=80, Tl=81, Pb=82, Bi=83, Po=84,
                   209.99, 222.02, 223.0, 226.0, 227.0, 232.04, 231.04,
                   //At=85, Rn=86, Fr=87, Ra=88, Ac=89, Th=90, Pa=91,
                   238.03, 237.0, 244.0, 243.0, 247.0, 247.0, 251.0,
                   //U=92, Np=93, Pu=94, Am=95, Cm=96, Bk=97, Cf=98,
                   252.0, 257.0, 258.0, 259.0, 262.0, 262.0, 262.0, 262.0
                   //Es=99,Fm=100,Md=101,No=102,Lr=103,Db=104,Jl=105,Rf=106
  };

  struct ElementMap {
    std::map<AtomType, Atom::Element> map_;
    ElementMap() {
      for (unsigned int i=0; element_data[i].at != AT_UNKNOWN; ++i) {
        map_[element_data[i].at]= element_data[i].e;
      }
    }
    Atom::Element get_element(AtomType at) const {
      IMP_check(map_.find(at) != map_.end(),
                "Unknown AtomType in get_element.",
                ValueException);
      return map_.find(at)->second;
    }
    bool get_has_element(AtomType at) const {
      return map_.find(at) != map_.end();
    }
    float get_mass(AtomType at) const {
      Atom::Element e = get_element(at);
      return mass[e];
    }
    unsigned int get_number_of_types() const {
      return map_.size();
    }
    void add_element(AtomType at, Atom::Element e) {
      map_[at]= e;
    }
  };

  ElementMap &get_element_map() {
    static ElementMap element_map;
    IMP_assert(element_map.get_number_of_types() > 40,
               "Map is not initialized: "
               << element_map.get_number_of_types());
    return element_map;
  }
}

Atom Atom::create(Particle *p, AtomType t) {
  p->add_attribute(get_type_key(), t.get_index());
  Hierarchy::create(p, Hierarchy::ATOM);
  Atom ret(p);
  //ret.set_atom_type(t);
  if (get_element_map().get_has_element(t)) {
    ret.set_element(get_element_map().get_element(t));
    ret.set_mass(get_element_map().get_mass(t));
  }
  return ret;
}

Atom Atom::create(Particle *p, Atom o) {
  p->add_attribute(get_type_key(), o.get_atom_type().get_index());
  Hierarchy::create(p, Hierarchy::ATOM);
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
  // ultimate the secondary info should be set from a
  // better source. But this is good enough for now.
  get_particle()->set_value(get_type_key(), t.get_index());
  if (get_element_map().get_has_element(t)) {
    set_element(get_element_map().get_element(t));
    set_mass(get_element_map().get_mass(t));
  }
}

IntKey Atom::get_type_key() {
  static IntKey k("atom_type");
  return k;
}

IntKey Atom::get_element_key() {
  static IntKey k("element");
  return k;
}

FloatKey Atom::get_mass_key() {
  static FloatKey k("atom_mass");
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
  } while (!Residue::is_instance_of(mhd.get_particle()));
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
  std::stringstream out;
  Particle *p = get_particle();
  out.setf(std::ios::left, std::ios::adjustfield);
  out.width(6);
  if (get_residue_type(*this) == UNK) {
    out << "HETATM";
  }
  else {
    out << "ATOM";
  }
  //7-11 : atom id
  out.setf(std::ios::right, std::ios::adjustfield);
  out.width(5);
  if (index==-1) {
    out << get_input_index();
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
  std::string atom_type = get_atom_type().get_string();
  if (atom_type.size()<4) {
    out << " ";
    out.width(3);
    out << atom_type;
  }
  else {
    out << atom_type;
  }
  // 17: skip the alternate indication position
  out.width(1);
  out << " ";

  // 18-20 : residue name
  out.width(3);
  out << get_residue_type(*this).get_string();
  //skip 21
  out.width(1);
  out << " ";
  // 22: chain identifier
  out << get_chain(*this);
  //23-26: residue number
  out.setf(std::ios::right, std::ios::adjustfield);
  out.width(4);
  out << get_residue_index(*this);
  //27: residue insertion code
  out.width(1);
  out << get_residue(*this).get_insertion_code();
  out.setf(std::ios::fixed, std::ios::floatfield);
  out << "   "; // skip 3 undefined positions (28-30)
  core::XYZ xyz= core::XYZ::cast(p);
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


AtomType add_atom_type(std::string name,
                       Atom::Element element) {
  IMP_check(!AtomType::get_key_exists(name),
            "An AtomType with that name already exists: "
            << name, ValueException);
  AtomType ret(name.c_str());
  get_element_map().add_element(ret, element);
  return ret;
}

IMPATOM_END_NAMESPACE
