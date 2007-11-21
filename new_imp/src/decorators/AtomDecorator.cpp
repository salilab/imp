/**
 *  \file AtomDecorator.cpp   \brief Simple atoms decorator.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <IMP/decorators/AtomDecorator.h>
#include <sstream>
#include <IMP/log.h>
#include <vector>
#include <limits>

namespace IMP
{



#define TYPE(STR) AtomType AtomDecorator::AT_##STR(#STR);
#define TYPE2(NAME, STR) AtomType AtomDecorator::AT_##NAME(#STR);


  TYPE(N);
  TYPE(H);
  TYPE(1H);
  TYPE(H1);
  TYPE(2H);
  TYPE(H2);
  TYPE(3H);
  TYPE(H3);
  TYPE(C);
  TYPE(O);
  TYPE(OXT);
  TYPE(CH3);

  TYPE(CA);
  TYPE(HA);
  TYPE(1HA);
  TYPE(2HA);

  TYPE(CB);
  TYPE(HB);
  TYPE(1HB);
  TYPE(2HB);
  TYPE(3HB);

  TYPE(CG);
  TYPE(CG1);
  TYPE(CG2);
  TYPE(HG);
  TYPE(1HG);
  TYPE(2HG);
  //TYPE(HG1",AtomDecorator::AT_HG1},
  TYPE(1HG1);
  TYPE(2HG1);
  TYPE(3HG1);
  TYPE(1HG2);
  TYPE(2HG2);
  TYPE(3HG2);
  TYPE(OG);
  TYPE(OG1);
  TYPE(SG);

  TYPE(CD);
  TYPE(CD1);
  TYPE(CD2);
  //TYPE(HD1",AtomDecorator::AT_HD1},
  //TYPE(HD2",AtomDecorator::AT_HD2},
  TYPE(HD);
  TYPE(1HD);
  TYPE(2HD);
  TYPE(3HD);
  TYPE(1HD1);
  TYPE(2HD1);
  TYPE(3HD1);
  TYPE(1HD2);
  TYPE(2HD2);
  TYPE(3HD2);
  TYPE(SD);
  TYPE(OD1);
  TYPE(OD2);
  TYPE(ND1);
  TYPE(ND2);

  TYPE(CE);
  TYPE(CE1);
  TYPE(CE2);
  TYPE(CE3);
  TYPE(HE);
  TYPE(1HE);
  TYPE(2HE);
  TYPE(3HE);
  //TYPE(HE1",AtomDecorator::AT_HE1},
  //TYPE(HE2",AtomDecorator::AT_HE2},
  //TYPE(HE3",AtomDecorator::AT_HE3},
  TYPE(1HE2);
  TYPE(2HE2);
  TYPE(OE1);
  TYPE(OE2);
  TYPE(NE);
  TYPE(NE1);
  TYPE(NE2);

  TYPE(CZ);
  TYPE(CZ2);
  TYPE(CZ3);
  TYPE(NZ);
  TYPE(HZ);
  TYPE(1HZ);
  TYPE(2HZ);
  TYPE(3HZ);
  //TYPE(HZ1",AtomDecorator::AT_HZ2},
  //TYPE(HZ2",AtomDecorator::AT_HZ2},
  //TYPE(HZ3",AtomDecorator::AT_HZ3},

  TYPE(CH2);
  TYPE(NH1);
  TYPE(NH2);
  TYPE(OH);
  TYPE(HH);

  TYPE(1HH1);
  TYPE(2HH1);
  TYPE(HH2);
  TYPE(1HH2);
  TYPE(2HH2);
  TYPE(2HH3);
  TYPE(3HH3);
  TYPE(1HH3);

  //TYPE(HH31);
  //TYPE(HH32);
  //TYPE(HH33);

  TYPE(P);
  TYPE(OP1);
  TYPE(OP2);
  TYPE2(O5p, O5*);
  TYPE2(C5p,C5*);
  TYPE2(H5pp, H5**);
  TYPE2(C4p, C4*);
  TYPE2(H4p, H4*);
  TYPE2(H5p, H5*);
  TYPE2(O4p, O4*);
  TYPE2(C1p, C1*);
  TYPE2(H1p, H1*);
  TYPE2(C3p, C3*);
  TYPE2(H3p, H3*);
  TYPE2(O3p, O3*);
  TYPE2(C2p, C2*);
  TYPE2(H2p, H2*);
  TYPE2(H2pp, H2**);
  TYPE2(O2p, O2*);
  TYPE2(HO2p, HO2*);
  TYPE(N9);
  TYPE(C8);
  TYPE(H8);
  TYPE(N7);
  TYPE(C5);
  TYPE(C4);
  TYPE(N3);
  TYPE(C2);
  TYPE(N1);
  TYPE(C6);
  TYPE(N6);
  TYPE(H61);
  TYPE(H62);
  TYPE(O6);

  TYPE(N2);
  TYPE(H21);
  TYPE(H22);

  TYPE(H6);
  TYPE(H5);
  TYPE(O2);
  TYPE(N4);
  TYPE(H41);
  TYPE(H42);

  TYPE(O4);
  TYPE(C7);
  TYPE(H71);
  TYPE(H72);
  TYPE(H73);

  TYPE(UNKNOWN);



IntKey AtomDecorator::element_key_;
FloatKey AtomDecorator::charge_key_;
  //FloatKey AtomDecorator::vdw_radius_key_;
FloatKey AtomDecorator::mass_key_;
IntKey AtomDecorator::type_key_;




void AtomDecorator::show(std::ostream &out, std::string prefix) const
{
  //out <<prefix << "Element:"<< get_element() << std::endl;
  out << prefix<< "Type: "<< get_type() << std::endl;
  XYZDecorator::show(out, prefix);
}


void AtomDecorator::set_type(AtomType t) {
  // ultimate the secondary info should be set from a 
  // better source. But this is good enough for now. 
  get_particle()->set_value(type_key_, t.get_index());
  /*std::string str=t.get_string();
  if (str.find("N") != std::string::npos) {
    set_element(N);
    set_van_der_waals_raidus(1.55);
  } else if (str.find("O") != std::string::npos) {
    set_element(O);
    set_van_der_waals_raidus(1.52);
  } else if (str.find("P") != std::string::npos) {
    set_element(P);
    set_van_der_waals_raidus(1.9);
  } else if (str.find("C") != std::string::npos) {
    set_element(C);
    set_van_der_waals_raidus(1.7);
  } else if (str.find("S") != std::string::npos) {
    set_element(S);
    set_van_der_waals_raidus(1.85);
  } else if (str.find("OH") != std::string::npos) {
    set_element(OH);
  } else if (str.find("H2O") != std::string::npos) {
    set_element(H2O);
  } else if (str.find("H") != std::string::npos) {
    set_element(H);
    set_van_der_waals_radius(1.20);
    }*/
}

IMP_DECORATOR_INITIALIZE(AtomDecorator, XYZDecorator,
                         {
                           element_key_= IntKey("atom element");
                           charge_key_= FloatKey("atom charge");
                           mass_key_= FloatKey("atom mass");
                           //vdw_radius_key_= FloatKey("atom vdw radius");
                           type_key_ = IntKey("atom type");
                         })

  /*void AtomDecorator::set_element(std::string cp)
{
  if (std::strchr(cp.c_str(), 'C') != NULL) set_element(C);
  else if (std::strchr(cp.c_str(),'N') != NULL) set_element( N);
  else if (std::strchr(cp.c_str(),'S') != NULL) set_element( S);
  else if (std::strchr(cp.c_str(),'O') != NULL) set_element( O);
  else if (std::strchr(cp.c_str(),'H') != NULL) set_element( H);
  else if (std::strstr(cp.c_str(),"FE") != NULL) set_element( FE);
  else if (std::strstr(cp.c_str(),"P") != NULL) set_element( P);
  else if (std::strstr(cp.c_str(),"PT") != NULL) set_element( PT);
  else {
    IMP_WARN("Unknown atom element string "<< cp.c_str() << std::endl);
  }
}

std::string AtomDecorator::get_element_string(int e)
{
  switch (e) {
  case C:
    return "C";
  case N:
    return "N";
  case H:
    return "H";
  case O:
    return "O";
  case S:
    return "S";
  case P:
    return "P";
  case FE:
    return "FE";
  case PT:
    return "PT";
  default:
    std::ostringstream oss;
    oss << "Element " << e;
    return oss.str().c_str();
  }
}

void AtomDecorator::set_type(std::string e)
{
  if (string_to_type_.find(e) != string_to_type_.end()) {
    set_type(string_to_type_.find(e)->second);
  } else {
    IMP_WARN("Unknown type string "<<
             e << " perhaps it is not space-padded");
  }
}

std::string AtomDecorator::get_type_string(Int t)
{
  if (type_to_string_.find(t) != type_to_string_.end()) {
    return type_to_string_.find(t)->second.c_str();
  } else if (t== AT_UNKNOWN) {
    return "Unknown";
  } else {
    IMP_failure("Unknown type passed for getting string",
                IndexException("Unknown type"));
    return "Unknown";
  }

}*/

} // namespace IMP
