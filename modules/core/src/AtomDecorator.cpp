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

#define TYPE_INIT(STR) AT_##STR= AtomType(#STR);
#define TYPE_INIT2(NAME, STR) AT_##NAME = AtomType(#STR);

#define TYPE_DEF(STR) AtomType AtomDecorator::AT_##STR;
#define TYPE_DEF2(NAME, STR) AtomType AtomDecorator::AT_##NAME;


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
                           TYPE_INIT(N);
                           TYPE_INIT(H);
                           TYPE_INIT(1H);
                           TYPE_INIT(H1);
                           TYPE_INIT(2H);
                           TYPE_INIT(H2);
                           TYPE_INIT(3H);
                           TYPE_INIT(H3);
                           TYPE_INIT(C);
                           TYPE_INIT(O);
                           TYPE_INIT(OXT);
                           TYPE_INIT(CH3);

                           TYPE_INIT(CA);
                           TYPE_INIT(HA);
                           TYPE_INIT(1HA);
                           TYPE_INIT(2HA);

                           TYPE_INIT(CB);
                           TYPE_INIT(HB);
                           TYPE_INIT(1HB);
                           TYPE_INIT(2HB);
                           TYPE_INIT(3HB);

                           TYPE_INIT(CG);
                           TYPE_INIT(CG1);
                           TYPE_INIT(CG2);
                           TYPE_INIT(HG);
                           TYPE_INIT(1HG);
                           TYPE_INIT(2HG);
                           //TYPE_INIT(HG1",AtomDecorator::AT_HG1},
                           TYPE_INIT(1HG1);
                           TYPE_INIT(2HG1);
                           TYPE_INIT(3HG1);
                           TYPE_INIT(1HG2);
                           TYPE_INIT(2HG2);
                           TYPE_INIT(3HG2);
                           TYPE_INIT(OG);
                           TYPE_INIT(OG1);
                           TYPE_INIT(SG);

                           TYPE_INIT(CD);
                           TYPE_INIT(CD1);
                           TYPE_INIT(CD2);
                           //TYPE_INIT(HD1",AtomDecorator::AT_HD1},
                           //TYPE_INIT(HD2",AtomDecorator::AT_HD2},
                           TYPE_INIT(HD);
                           TYPE_INIT(1HD);
                           TYPE_INIT(2HD);
                           TYPE_INIT(3HD);
                           TYPE_INIT(1HD1);
                           TYPE_INIT(2HD1);
                           TYPE_INIT(3HD1);
                           TYPE_INIT(1HD2);
                           TYPE_INIT(2HD2);
                           TYPE_INIT(3HD2);
                           TYPE_INIT(SD);
                           TYPE_INIT(OD1);
                           TYPE_INIT(OD2);
                           TYPE_INIT(ND1);
                           TYPE_INIT(ND2);

                           TYPE_INIT(CE);
                           TYPE_INIT(CE1);
                           TYPE_INIT(CE2);
                           TYPE_INIT(CE3);
                           TYPE_INIT(HE);
                           TYPE_INIT(1HE);
                           TYPE_INIT(2HE);
                           TYPE_INIT(3HE);
                           //TYPE_INIT(HE1",AtomDecorator::AT_HE1},
                           //TYPE_INIT(HE2",AtomDecorator::AT_HE2},
                           //TYPE_INIT(HE3",AtomDecorator::AT_HE3},
                           TYPE_INIT(1HE2);
                           TYPE_INIT(2HE2);
                           TYPE_INIT(OE1);
                           TYPE_INIT(OE2);
                           TYPE_INIT(NE);
                           TYPE_INIT(NE1);
                           TYPE_INIT(NE2);

                           TYPE_INIT(CZ);
                           TYPE_INIT(CZ2);
                           TYPE_INIT(CZ3);
                           TYPE_INIT(NZ);
                           TYPE_INIT(HZ);
                           TYPE_INIT(1HZ);
                           TYPE_INIT(2HZ);
                           TYPE_INIT(3HZ);
                           //TYPE_INIT(HZ1",AtomDecorator::AT_HZ2},
                           //TYPE_INIT(HZ2",AtomDecorator::AT_HZ2},
                           //TYPE_INIT(HZ3",AtomDecorator::AT_HZ3},

                           TYPE_INIT(CH2);
                           TYPE_INIT(NH1);
                           TYPE_INIT(NH2);
                           TYPE_INIT(OH);
                           TYPE_INIT(HH);

                           TYPE_INIT(1HH1);
                           TYPE_INIT(2HH1);
                           TYPE_INIT(HH2);
                           TYPE_INIT(1HH2);
                           TYPE_INIT(2HH2);
                           TYPE_INIT(2HH3);
                           TYPE_INIT(3HH3);
                           TYPE_INIT(1HH3);

                           //TYPE_INIT(HH31);
                           //TYPE_INIT(HH32);
                           //TYPE_INIT(HH33);

                           TYPE_INIT(P);
                           TYPE_INIT(OP1);
                           TYPE_INIT(OP2);
                           TYPE_INIT2(O5p, O5*);
                           TYPE_INIT2(C5p,C5*);
                           TYPE_INIT2(H5pp, H5**);
                           TYPE_INIT2(C4p, C4*);
                           TYPE_INIT2(H4p, H4*);
                           TYPE_INIT2(H5p, H5*);
                           TYPE_INIT2(O4p, O4*);
                           TYPE_INIT2(C1p, C1*);
                           TYPE_INIT2(H1p, H1*);
                           TYPE_INIT2(C3p, C3*);
                           TYPE_INIT2(H3p, H3*);
                           TYPE_INIT2(O3p, O3*);
                           TYPE_INIT2(C2p, C2*);
                           TYPE_INIT2(H2p, H2*);
                           TYPE_INIT2(H2pp, H2**);
                           TYPE_INIT2(O2p, O2*);
                           TYPE_INIT2(HO2p, HO2*);
                           TYPE_INIT(N9);
                           TYPE_INIT(C8);
                           TYPE_INIT(H8);
                           TYPE_INIT(N7);
                           TYPE_INIT(C5);
                           TYPE_INIT(C4);
                           TYPE_INIT(N3);
                           TYPE_INIT(C2);
                           TYPE_INIT(N1);
                           TYPE_INIT(C6);
                           TYPE_INIT(N6);
                           TYPE_INIT(H61);
                           TYPE_INIT(H62);
                           TYPE_INIT(O6);

                           TYPE_INIT(N2);
                           TYPE_INIT(H21);
                           TYPE_INIT(H22);

                           TYPE_INIT(H6);
                           TYPE_INIT(H5);
                           TYPE_INIT(O2);
                           TYPE_INIT(N4);
                           TYPE_INIT(H41);
                           TYPE_INIT(H42);

                           TYPE_INIT(O4);
                           TYPE_INIT(C7);
                           TYPE_INIT(H71);
                           TYPE_INIT(H72);
                           TYPE_INIT(H73);

                           TYPE_INIT(UNKNOWN);
                         })

IMPCORE_END_NAMESPACE
