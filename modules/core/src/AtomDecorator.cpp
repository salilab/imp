/**
 *  \file AtomDecorator.cpp   \brief Simple atoms decorator.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/AtomDecorator.h>

#include <IMP/log.h>

#include <sstream>
#include <vector>
#include <limits>

IMPCORE_BEGIN_NAMESPACE

#define TYPE_DEF(STR) AtomType AtomDecorator::AT_##STR(#STR);
#define TYPE_DEF2(NAME, STR) AtomType AtomDecorator::AT_##NAME(#STR);


IntKey AtomDecorator::element_key_;
FloatKey AtomDecorator::charge_key_;
FloatKey AtomDecorator::mass_key_;
IntKey AtomDecorator::type_key_;
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
  out << prefix<< "Type: "<< get_type() << std::endl;
  XYZDecorator::show(out, prefix);
}


void AtomDecorator::set_type(AtomType t)
{
  // ultimate the secondary info should be set from a
  // better source. But this is good enough for now.
  get_particle()->set_value(type_key_, t.get_index());
}

IMP_DECORATOR_INITIALIZE(AtomDecorator, XYZDecorator,
                         {
                           element_key_= IntKey("atom element");
                           charge_key_= FloatKey("atom charge");
                           mass_key_= FloatKey("atom mass");
                           //vdw_radius_key_= FloatKey("atom vdw radius");
                           type_key_ = IntKey("atom type");
                         })

IMPCORE_END_NAMESPACE
