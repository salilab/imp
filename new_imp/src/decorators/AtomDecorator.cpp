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

struct AtomData {
  const char *str;
  AtomDecorator::Type type;
  AtomDecorator::Element element;
};

static double nan__=std::numeric_limits<double>::signaling_NaN();


static double vdwradii_[]={0, 1.70, 1.55, 1.20, 1.52, 1.85, 1.10, 1.75,
                           nan__, nan__, nan__, nan__
                          };


static AtomData atom_name_data_[]= {
  {"N",AtomDecorator::AT_N, AtomDecorator::N},
  {"H",AtomDecorator::AT_H, AtomDecorator::H},
  {"1H",AtomDecorator::AT_1H, AtomDecorator::H},
  {"H1",AtomDecorator::AT_1H, AtomDecorator::H},
  {"2H",AtomDecorator::AT_2H, AtomDecorator::H},
  {"H2",AtomDecorator::AT_2H, AtomDecorator::H},
  {"3H",AtomDecorator::AT_3H, AtomDecorator::H},
  {"H3",AtomDecorator::AT_3H, AtomDecorator::H},
  {"C",AtomDecorator::AT_C, AtomDecorator::C},
  {"O",AtomDecorator::AT_O, AtomDecorator::O},
  {"OXT",AtomDecorator::AT_OXT, AtomDecorator::O},
  {"CH3",AtomDecorator::AT_CH3, AtomDecorator::C},

  {"CA",AtomDecorator::AT_CA, AtomDecorator::C},
  {"HA",AtomDecorator::AT_HA, AtomDecorator::H},
  {"1HA",AtomDecorator::AT_1HA, AtomDecorator::H},
  {"HA1",AtomDecorator::AT_1HA, AtomDecorator::H},
  {"2HA",AtomDecorator::AT_2HA, AtomDecorator::H},
  {"HA2",AtomDecorator::AT_2HA, AtomDecorator::H},

  {"CB",AtomDecorator::AT_CB, AtomDecorator::C},
  {"HB",AtomDecorator::AT_HB, AtomDecorator::H},
  {"1HB",AtomDecorator::AT_1HB, AtomDecorator::H},
  {"HB1",AtomDecorator::AT_1HB, AtomDecorator::H},
  {"2HB", AtomDecorator::AT_2HB, AtomDecorator::H},
  {"HB2", AtomDecorator::AT_2HB, AtomDecorator::H},
  {"3HB", AtomDecorator::AT_3HB, AtomDecorator::H},
  {"HB3", AtomDecorator::AT_3HB, AtomDecorator::H},

  {"CG",AtomDecorator::AT_CG, AtomDecorator::C},
  {"CG1",AtomDecorator::AT_CG1, AtomDecorator::C},
  {"CG2",AtomDecorator::AT_CG2, AtomDecorator::C},
  {"HG",AtomDecorator::AT_HG, AtomDecorator::H},
  {"1HG", AtomDecorator::AT_1HG, AtomDecorator::H},
  {"HG1", AtomDecorator::AT_1HG, AtomDecorator::H},
  {"2HG", AtomDecorator::AT_2HG, AtomDecorator::H},
  {"HG2", AtomDecorator::AT_2HG, AtomDecorator::H},
  //{"HG1",AtomDecorator::AT_HG1},
  {"1HG1",AtomDecorator::AT_1HG1, AtomDecorator::H},
  {"HG11",AtomDecorator::AT_1HG1, AtomDecorator::H},
  {"2HG1",AtomDecorator::AT_2HG1, AtomDecorator::H},
  {"HG12",AtomDecorator::AT_2HG1, AtomDecorator::H},
  {"3HG1",AtomDecorator::AT_3HG1, AtomDecorator::H},
  {"HG13",AtomDecorator::AT_3HG1, AtomDecorator::H},
  {"1HG2",AtomDecorator::AT_1HG2, AtomDecorator::H},
  {"HG21",AtomDecorator::AT_1HG2, AtomDecorator::H},
  {"2HG2",AtomDecorator::AT_2HG2, AtomDecorator::H},
  {"HG22",AtomDecorator::AT_2HG2, AtomDecorator::H},
  {"3HG2",AtomDecorator::AT_3HG2, AtomDecorator::H},
  {"HG23",AtomDecorator::AT_3HG2, AtomDecorator::H},
  {"OG",AtomDecorator::AT_OG, AtomDecorator::O},
  {"OG1",AtomDecorator::AT_OG1, AtomDecorator::O},
  {"SG",AtomDecorator::AT_SG, AtomDecorator::S},

  {"CD",AtomDecorator::AT_CD, AtomDecorator::C},
  {"CD1",AtomDecorator::AT_CD1, AtomDecorator::C},
  {"CD2",AtomDecorator::AT_CD2, AtomDecorator::C},
  //{"HD1",AtomDecorator::AT_HD1},
  //{"HD2",AtomDecorator::AT_HD2},
  {"HD",AtomDecorator::AT_HD, AtomDecorator::H},
  {"1HD",AtomDecorator::AT_1HD, AtomDecorator::H},
  {"HD1",AtomDecorator::AT_1HD, AtomDecorator::H},
  {"2HD",AtomDecorator::AT_2HD, AtomDecorator::H},
  {"HD2",AtomDecorator::AT_2HD, AtomDecorator::H},
  {"3HD",AtomDecorator::AT_3HD, AtomDecorator::H},
  {"HD3",AtomDecorator::AT_3HD, AtomDecorator::H},
  {"1HD1",AtomDecorator::AT_1HD1, AtomDecorator::H},
  {"HD11",AtomDecorator::AT_1HD1, AtomDecorator::H},
  {"2HD1",AtomDecorator::AT_2HD1, AtomDecorator::H},
  {"HD12",AtomDecorator::AT_2HD1, AtomDecorator::H},
  {"3HD1",AtomDecorator::AT_3HD1, AtomDecorator::H},
  {"HD13",AtomDecorator::AT_3HD1, AtomDecorator::H},
  {"1HD2",AtomDecorator::AT_1HD2, AtomDecorator::H},
  {"HD21",AtomDecorator::AT_1HD2, AtomDecorator::H},
  {"2HD2",AtomDecorator::AT_2HD2, AtomDecorator::H},
  {"HD22",AtomDecorator::AT_2HD2, AtomDecorator::H},
  {"3HD2",AtomDecorator::AT_3HD2, AtomDecorator::H},
  {"HD23",AtomDecorator::AT_3HD2, AtomDecorator::H},
  {"SD",AtomDecorator::AT_SD, AtomDecorator::S},
  {"OD1",AtomDecorator::AT_OD1, AtomDecorator::O},
  {"OD2",AtomDecorator::AT_OD2, AtomDecorator::O},
  {"ND1",AtomDecorator::AT_ND1, AtomDecorator::N},
  {"ND2",AtomDecorator::AT_ND2, AtomDecorator::N},

  {"CE",AtomDecorator::AT_CE, AtomDecorator::C},
  {"CE1",AtomDecorator::AT_CE1, AtomDecorator::C},
  {"CE2",AtomDecorator::AT_CE2, AtomDecorator::C},
  {"CE3",AtomDecorator::AT_CE3, AtomDecorator::C},
  {"HE",AtomDecorator::AT_HE, AtomDecorator::H},
  {"1HE",AtomDecorator::AT_1HE, AtomDecorator::H},
  {"HE1",AtomDecorator::AT_1HE, AtomDecorator::H},
  {"2HE",AtomDecorator::AT_2HE, AtomDecorator::H},
  {"HE2",AtomDecorator::AT_2HE, AtomDecorator::H},
  {"3HE",AtomDecorator::AT_3HE, AtomDecorator::H},
  {"HE3",AtomDecorator::AT_3HE, AtomDecorator::H},
  //{"HE1",AtomDecorator::AT_HE1},
  //{"HE2",AtomDecorator::AT_HE2},
  //{"HE3",AtomDecorator::AT_HE3},
  {"1HE2",AtomDecorator::AT_1HE2, AtomDecorator::H},
  {"HE21",AtomDecorator::AT_1HE2, AtomDecorator::H},
  {"2HE2",AtomDecorator::AT_2HE2, AtomDecorator::H},
  {"HE22",AtomDecorator::AT_2HE2, AtomDecorator::H},
  {"OE1",AtomDecorator::AT_OE1, AtomDecorator::O},
  {"OE2",AtomDecorator::AT_OE2, AtomDecorator::O},
  {"NE",AtomDecorator::AT_NE, AtomDecorator::N},
  {"NE1",AtomDecorator::AT_NE1, AtomDecorator::N},
  {"NE2",AtomDecorator::AT_NE2, AtomDecorator::N},

  {"CZ",AtomDecorator::AT_CZ, AtomDecorator::C},
  {"CZ2",AtomDecorator::AT_CZ2, AtomDecorator::C},
  {"CZ3",AtomDecorator::AT_CZ3, AtomDecorator::C},
  {"NZ",AtomDecorator::AT_NZ, AtomDecorator::N},
  {"HZ",AtomDecorator::AT_HZ, AtomDecorator::H},
  {"1HZ",AtomDecorator::AT_1HZ, AtomDecorator::H},
  {"HZ1",AtomDecorator::AT_1HZ, AtomDecorator::H},
  {"2HZ",AtomDecorator::AT_2HZ, AtomDecorator::H},
  {"HZ2",AtomDecorator::AT_2HZ, AtomDecorator::H},
  {"3HZ",AtomDecorator::AT_3HZ, AtomDecorator::H},
  {"HZ3",AtomDecorator::AT_3HZ, AtomDecorator::H},
  //{"HZ1",AtomDecorator::AT_HZ2},
  //{"HZ2",AtomDecorator::AT_HZ2},
  //{"HZ3",AtomDecorator::AT_HZ3},

  {"CH2",AtomDecorator::AT_CH2, AtomDecorator::C},
  {"NH1",AtomDecorator::AT_NH1, AtomDecorator::N},
  {"NH2",AtomDecorator::AT_NH2, AtomDecorator::N},
  {"OH",AtomDecorator::AT_OH, AtomDecorator::O},
  {"HH",AtomDecorator::AT_HH, AtomDecorator::H},

  {"1HH1", AtomDecorator::AT_1HH1, AtomDecorator::H},
  {"HH11", AtomDecorator::AT_1HH1, AtomDecorator::H},
  {"2HH1", AtomDecorator::AT_2HH1, AtomDecorator::H},
  {"HH12", AtomDecorator::AT_2HH1, AtomDecorator::H},
  {"HH2",AtomDecorator::AT_HH2, AtomDecorator::H},
  {"1HH2", AtomDecorator::AT_1HH2, AtomDecorator::H},
  {"HH21", AtomDecorator::AT_1HH2, AtomDecorator::H},
  {"2HH2", AtomDecorator::AT_2HH2, AtomDecorator::H},
  {"HH22", AtomDecorator::AT_2HH2, AtomDecorator::H},
  {"HH", AtomDecorator::AT_1HH2, AtomDecorator::H},

  {"HH31", AtomDecorator::AT_1HH3, AtomDecorator::H},
  {"HH32", AtomDecorator::AT_2HH3, AtomDecorator::H},
  {"HH33", AtomDecorator::AT_3HH3, AtomDecorator::H},

  {"P ", AtomDecorator::AT_P, AtomDecorator::P},
  {"O1P", AtomDecorator::AT_OP1, AtomDecorator::O},
  {"O2P", AtomDecorator::AT_OP2, AtomDecorator::O},
  {"O5*", AtomDecorator::AT_O5p, AtomDecorator::O},
  {"C5*", AtomDecorator::AT_H5p, AtomDecorator::C},
  {"H5**", AtomDecorator::AT_H5pp, AtomDecorator::H},
  {"C4*", AtomDecorator::AT_C4p, AtomDecorator::C},
  {"H4*", AtomDecorator::AT_H4p, AtomDecorator::H},
  {"O4*", AtomDecorator::AT_O4p, AtomDecorator::O},
  {"C1*", AtomDecorator::AT_C1p, AtomDecorator::C},
  {"H1*", AtomDecorator::AT_H1p, AtomDecorator::H},
  {"C3*", AtomDecorator::AT_C3p, AtomDecorator::C},
  {"H3*", AtomDecorator::AT_H3p, AtomDecorator::H},
  {"O3*", AtomDecorator::AT_O3p, AtomDecorator::O},
  {"C2*", AtomDecorator::AT_C2p, AtomDecorator::C},
  {"H2*", AtomDecorator::AT_H2p, AtomDecorator::H},
  {"H2**", AtomDecorator::AT_H2pp, AtomDecorator::H},
  {"O2*", AtomDecorator::AT_O2p, AtomDecorator::O},
  {"HO2*", AtomDecorator::AT_HO2p, AtomDecorator::O},
  {"N9", AtomDecorator::AT_N9, AtomDecorator::N},
  {"C8", AtomDecorator::AT_C8, AtomDecorator::C},
  {"H8", AtomDecorator::AT_H8, AtomDecorator::H},
  {"N7", AtomDecorator::AT_N7, AtomDecorator::N},
  {"C5", AtomDecorator::AT_C5, AtomDecorator::C},
  {"C4", AtomDecorator::AT_C4, AtomDecorator::C},
  {"N3", AtomDecorator::AT_N3, AtomDecorator::N},
  {"C2", AtomDecorator::AT_C2, AtomDecorator::C},
  {"H2", AtomDecorator::AT_H2, AtomDecorator::H},
  {"N1", AtomDecorator::AT_N1, AtomDecorator::N},
  {"C6", AtomDecorator::AT_C6, AtomDecorator::C},
  {"N6", AtomDecorator::AT_N6, AtomDecorator::N},
  {"H61", AtomDecorator::AT_H61, AtomDecorator::H},
  {"H62", AtomDecorator::AT_H62, AtomDecorator::H},
  {"O6", AtomDecorator::AT_O6, AtomDecorator::O},
  {"H1", AtomDecorator::AT_H1, AtomDecorator::H},
  {"N2", AtomDecorator::AT_N2, AtomDecorator::N},
  {"H21", AtomDecorator::AT_H21, AtomDecorator::H},
  {"H22", AtomDecorator::AT_H22, AtomDecorator::H},

  {"H6", AtomDecorator::AT_H6, AtomDecorator::H},
  {"H5", AtomDecorator::AT_H5, AtomDecorator::H},
  {"O2", AtomDecorator::AT_O2, AtomDecorator::O},
  {"N4", AtomDecorator::AT_N4, AtomDecorator::N},
  {"H41", AtomDecorator::AT_H41, AtomDecorator::H},
  {"H42", AtomDecorator::AT_H42,AtomDecorator::H},
  {"H3", AtomDecorator::AT_H3, AtomDecorator::H},
  {"O4", AtomDecorator::AT_O4, AtomDecorator::O},
  {"C7", AtomDecorator::AT_C7, AtomDecorator::C},
  {"H71", AtomDecorator::AT_H71, AtomDecorator::H},
  {"H72", AtomDecorator::AT_H72,AtomDecorator::H},
  {"H73", AtomDecorator::AT_H73,AtomDecorator::H},

  {"UNKN", AtomDecorator::AT_UNKNOWN, AtomDecorator::UNKNOWN_ELEMENT}
};

static std::map<std::string, AtomDecorator::Type> string_to_type_;
static std::map<AtomDecorator::Type, std::string> type_to_string_;








bool AtomDecorator::keys_initialized_=false;
IntKey AtomDecorator::element_key_;
FloatKey AtomDecorator::charge_key_;
FloatKey AtomDecorator::formal_charge_key_;
IntKey AtomDecorator::type_key_;




void AtomDecorator::show(std::ostream &out) const
{
  out <<"Element:"<< get_element() << std::endl;
  out <<"Type: "<< get_type() << std::endl;
}





void AtomDecorator::initialize_static_data()
{
  if (keys_initialized_) return;
  else {
    element_key_= IntKey("atom element");
    charge_key_= FloatKey("atom charge");
    formal_charge_key_= FloatKey("atom formal charge");
    type_key_ = IntKey("atom type");

    int i=0; ;
    while (atom_name_data_[i].type != AT_UNKNOWN) {
      string_to_type_[atom_name_data_[i].str]
      = atom_name_data_[i].type;
      if (type_to_string_.find(atom_name_data_[i].type)
          == type_to_string_.end()) {
        type_to_string_[atom_name_data_[i].type]
        = atom_name_data_[i].str;
      }
    }
    keys_initialized_=true;
  }
}

void AtomDecorator::set_element(std::string cp)
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

const char* AtomDecorator::get_element_string(Element e)
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
    IMP_failure("Some element type is not handled or unknown"
                "element type passed", IndexException("Unknown element"));
    return "unknown";
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

const char * AtomDecorator::get_type_string(Type t)
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

}

} // namespace IMP
