/**
 *  \file AtomDecorator.cpp   \brief Simple atoms decorator.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/atom/AtomDecorator.h>
#include <IMP/atom/MolecularHierarchyDecorator.h>
#include <IMP/atom/ChainDecorator.h>

#include <IMP/log.h>

#include <sstream>
#include <vector>
#include <limits>

IMPATOM_BEGIN_NAMESPACE

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

namespace {
  struct ElementData {
    AtomType at;
    AtomDecorator::Element e;
  };

  ElementData element_data[]=
    {{AT_N, AtomDecorator::N},
     {AT_H, AtomDecorator::H},
     {AT_1H, AtomDecorator::H},
     {AT_2H, AtomDecorator::H},
     {AT_3H, AtomDecorator::H},
     {AT_C, AtomDecorator::C},
     {AT_O, AtomDecorator::O},
     {AT_OXT, AtomDecorator::O},
     {AT_CH3, AtomDecorator::C},

     {AT_CA, AtomDecorator::C},
     {AT_HA, AtomDecorator::H},
     {AT_1HA, AtomDecorator::H},
     {AT_2HA, AtomDecorator::H},

     {AT_CB, AtomDecorator::C},
     {AT_HB, AtomDecorator::H},
     {AT_1HB, AtomDecorator::H},
     { AT_2HB, AtomDecorator::H},
     { AT_3HB, AtomDecorator::H},

     {AT_CG, AtomDecorator::C},
     {AT_CG1, AtomDecorator::C},
     {AT_CG2, AtomDecorator::C},
     {AT_HG, AtomDecorator::H},
     { AT_1HG, AtomDecorator::H},
     { AT_2HG, AtomDecorator::H},
     //{AT_HG1},
     {AT_1HG1, AtomDecorator::H},
     {AT_2HG1, AtomDecorator::H},
     {AT_3HG1, AtomDecorator::H},
     {AT_1HG2, AtomDecorator::H},
     {AT_2HG2, AtomDecorator::H},
     {AT_3HG2, AtomDecorator::H},
     {AT_OG, AtomDecorator::O},
     {AT_OG1, AtomDecorator::O},
     {AT_SG, AtomDecorator::S},

     {AT_CD, AtomDecorator::C},
     {AT_CD1, AtomDecorator::C},
     {AT_CD2, AtomDecorator::C},
     //{AT_HD1},
     //{AT_HD2},
     {AT_HD, AtomDecorator::H},
     {AT_1HD, AtomDecorator::H},
     {AT_2HD, AtomDecorator::H},
     {AT_3HD, AtomDecorator::H},
     {AT_1HD1, AtomDecorator::H},
     {AT_2HD1, AtomDecorator::H},
     {AT_3HD1, AtomDecorator::H},
     {AT_1HD2, AtomDecorator::H},
     {AT_2HD2, AtomDecorator::H},
     {AT_3HD2, AtomDecorator::H},
     {AT_SD, AtomDecorator::S},
     {AT_OD1, AtomDecorator::O},
     {AT_OD2, AtomDecorator::O},
     {AT_ND1, AtomDecorator::N},
     {AT_ND2, AtomDecorator::N},

     {AT_CE, AtomDecorator::C},
     {AT_CE1, AtomDecorator::C},
     {AT_CE2, AtomDecorator::C},
     {AT_CE3, AtomDecorator::C},
     {AT_HE, AtomDecorator::H},
     {AT_1HE, AtomDecorator::H},{AT_1HE, AtomDecorator::H},
     {AT_2HE, AtomDecorator::H},{AT_2HE, AtomDecorator::H},
     {AT_3HE, AtomDecorator::H},{AT_3HE, AtomDecorator::H},
     //{AT_HE1},
     //{AT_HE2},
     //{AT_HE3},
     {AT_1HE2, AtomDecorator::H},{AT_1HE2, AtomDecorator::H},
     {AT_2HE2, AtomDecorator::H},{AT_2HE2, AtomDecorator::H},
     {AT_OE1, AtomDecorator::O},
     {AT_OE2, AtomDecorator::O},
     {AT_NE, AtomDecorator::N},
     {AT_NE1, AtomDecorator::N},
     {AT_NE2, AtomDecorator::N},

     {AT_CZ, AtomDecorator::C},
     {AT_CZ2, AtomDecorator::C},
     {AT_CZ3, AtomDecorator::C},
     {AT_NZ, AtomDecorator::N},
     {AT_HZ, AtomDecorator::H},
     {AT_1HZ, AtomDecorator::H},
     {AT_2HZ, AtomDecorator::H},
     {AT_3HZ, AtomDecorator::H},
     //{AT_HZ2},
     //{AT_HZ2},
     //{AT_HZ3},

     {AT_CH2, AtomDecorator::C},
     {AT_NH1, AtomDecorator::N},
     {AT_NH2, AtomDecorator::N},
     {AT_OH, AtomDecorator::O},
     {AT_HH, AtomDecorator::H},

     { AT_1HH1, AtomDecorator::H},
     { AT_2HH1, AtomDecorator::H},
     {AT_HH2, AtomDecorator::H},
     { AT_1HH2, AtomDecorator::H},
     { AT_2HH2, AtomDecorator::H},
     { AT_1HH2, AtomDecorator::H},

     { AT_1HH3, AtomDecorator::H},
     { AT_2HH3, AtomDecorator::H},
     { AT_3HH3, AtomDecorator::H},

     { AT_P, AtomDecorator::P},
     { AT_OP1, AtomDecorator::O},
     { AT_OP2, AtomDecorator::O},
     { AT_O5p, AtomDecorator::O},
     { AT_H5p, AtomDecorator::C},
     { AT_H5pp, AtomDecorator::H},
     { AT_C4p, AtomDecorator::C},
     { AT_H4p, AtomDecorator::H},
     { AT_O4p, AtomDecorator::O},
     { AT_C1p, AtomDecorator::C},
     { AT_H1p, AtomDecorator::H},
     { AT_C3p, AtomDecorator::C},
     { AT_H3p, AtomDecorator::H},
     { AT_O3p, AtomDecorator::O},
     { AT_C2p, AtomDecorator::C},
     { AT_H2p, AtomDecorator::H},
     { AT_H2pp, AtomDecorator::H},
     { AT_O2p, AtomDecorator::O},
     { AT_HO2p, AtomDecorator::O},
     { AT_N9, AtomDecorator::N},
     { AT_C8, AtomDecorator::C},
     { AT_H8, AtomDecorator::H},
     { AT_N7, AtomDecorator::N},
     { AT_C5, AtomDecorator::C},
     { AT_C4, AtomDecorator::C},
     { AT_N3, AtomDecorator::N},
     { AT_C2, AtomDecorator::C},
     { AT_H2, AtomDecorator::H},
     { AT_N1, AtomDecorator::N},
     { AT_C6, AtomDecorator::C},
     { AT_N6, AtomDecorator::N},
     { AT_H61, AtomDecorator::H},
     { AT_H62, AtomDecorator::H},
     { AT_O6, AtomDecorator::O},
     { AT_H1, AtomDecorator::H},
     { AT_N2, AtomDecorator::N},
     { AT_H21, AtomDecorator::H},
     { AT_H22, AtomDecorator::H},

     { AT_H6, AtomDecorator::H},
     { AT_H5, AtomDecorator::H},
     { AT_O2, AtomDecorator::O},
     { AT_N4, AtomDecorator::N},
     { AT_H41, AtomDecorator::H},
     { AT_H42,AtomDecorator::H},
     { AT_H3, AtomDecorator::H},
     { AT_O4, AtomDecorator::O},
     { AT_C7, AtomDecorator::C},
     { AT_H71, AtomDecorator::H},
     { AT_H72,AtomDecorator::H},
     { AT_H73,AtomDecorator::H},
     { AT_UNKNOWN, AtomDecorator::UNKNOWN_ELEMENT}};

  struct ElementMap {
    std::map<AtomType, AtomDecorator::Element> map_;
    ElementMap() {
      for (unsigned int i=0; element_data[i].at != AT_UNKNOWN; ++i) {
        map_[element_data[i].at]= element_data[i].e;
      }
    }
    AtomDecorator::Element get_element(AtomType at) const {
      IMP_check(map_.find(at) != map_.end(),
                "Unknown AtomType in get_element.",
                ValueException);
      return map_.find(at)->second;
    }
    bool get_has_element(AtomType at) const {
      return map_.find(at) != map_.end();
    }
    unsigned int get_number_of_types() const {
      return map_.size();
    }
    void add_element(AtomType at, AtomDecorator::Element e) {
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

AtomDecorator AtomDecorator::create(Particle *p, AtomType t,
                                    const algebra::Vector3D &v) {
  p->add_attribute(get_type_key(), t.get_index());
  XYZDecorator::create(p, v);
  MolecularHierarchyDecorator::create(p, MolecularHierarchyDecorator::ATOM);
  AtomDecorator ret(p);
  return ret;
}

AtomDecorator AtomDecorator::create(Particle *p, AtomDecorator o) {
  XYZDecorator::create(p, o.get_coordinates());
  p->add_attribute(get_type_key(), o.get_type().get_index());
  MolecularHierarchyDecorator::create(p, MolecularHierarchyDecorator::ATOM);
  AtomDecorator ret(p);
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
  if (get_element_map().get_has_element(t)) {
    set_element(get_element_map().get_element(t));
  }
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

int get_residue_index(AtomDecorator d) {
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

char get_chain(AtomDecorator d) {
  ResidueDecorator rd = get_residue(d);
  MolecularHierarchyDecorator mhd(rd.get_particle());
  do {
    mhd= mhd.get_parent();
    if (mhd == MolecularHierarchyDecorator()) {
      throw InvalidStateException("Residue is not the child of a chain");
    }
  } while (mhd.get_type() != MolecularHierarchyDecorator::CHAIN);
  ChainDecorator cd(mhd.get_particle());
  return cd.get_id();
}


std::string AtomDecorator::get_pdb_string(int index) {
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
  std::string atom_type = get_type().get_string();
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


AtomType add_atom_type(std::string name,
                       AtomDecorator::Element element) {
  IMP_check(!AtomType::get_key_exists(name),
            "An AtomType with that name already exists: "
            << name, ValueException);
  AtomType ret(name.c_str());
  get_element_map().add_element(ret, element);
  return ret;
}

IMPATOM_END_NAMESPACE
