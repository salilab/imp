/**
 *  \file AtomDecorator.h     \brief Simple atom decorator.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_ATOM_DECORATOR_H
#define __IMP_ATOM_DECORATOR_H

#include "../Particle.h"
#include "../Model.h"
#include "utility.h"
#include "XYZDecorator.h"
#include <vector>
#include <deque>

namespace IMP
{


//! A single name decorator.
/**
   At some point the Types and elements need to be expanded to be more complete.
   Elements can be done exhaustively, but we need some sort of dynamic 
   mechanism for the types. This could be done by having an 
   add_type(string) method which adds a string name and assigns it to the 
   next unused type int available.
 */
  class IMPDLLEXPORT AtomDecorator: public XYZDecorator
{
  IMP_DECORATOR(AtomDecorator, XYZDecorator,
                return p->has_attribute(type_key_),
                { p->add_attribute(type_key_, AT_UNKNOWN.get_index()); });
protected:
  static IntKey element_key_;
  static FloatKey charge_key_;
  //static FloatKey vdw_radius_key_;
  static FloatKey mass_key_;
  static IntKey type_key_;

 public:
  //! The various elements currently supported
  /**
     The enum just provides mapping from a name to an atomic number. It
     should probably be expanded to be complete at some point.
   */
  enum Element {UNKNOWN_ELEMENT=0, OH=-1, H2O=-2,
                C=6,N=7,H=1, O=8, S=16,P=15,FE=26, PT=78
  };

  //! The types of PDB atoms supported
  /** \note each static must be on a separate line because of MSVC bug C2487:
            see http://support.microsoft.com/kb/127900/
   */
  static AtomType AT_UNKNOWN;
  static AtomType AT_N;
  static AtomType AT_CA;
  static AtomType AT_C;
  static AtomType AT_O;
  static AtomType AT_H;
  static AtomType AT_1H;
  static AtomType AT_2H;
  static AtomType AT_3H;
  static AtomType AT_HA;
  static AtomType AT_1HA;
  static AtomType AT_2HA;
  static AtomType AT_CB;
  static AtomType AT_HB;
  static AtomType AT_1HB;
  static AtomType AT_2HB;
  static AtomType AT_3HB;
  static AtomType AT_OXT;
  static AtomType AT_CH3;
  static AtomType AT_CG;
  static AtomType AT_CG1;
  static AtomType AT_CG2;
  static AtomType AT_HG;
  static AtomType AT_1HG;
  static AtomType AT_2HG;
  // static AtomType AT_HG1;
  static AtomType AT_1HG1;
  static AtomType AT_2HG1;
  static AtomType AT_3HG1;
  static AtomType AT_1HG2;
  static AtomType AT_2HG2;
  static AtomType AT_3HG2;
  static AtomType AT_OG;
  static AtomType AT_OG1;
  static AtomType AT_SG;
  // static AtomType AT_HD1;
  // static AtomType AT_HD2;
  static AtomType AT_CD;
  static AtomType AT_CD1;
  static AtomType AT_CD2;
  static AtomType AT_HD;
  static AtomType AT_1HD;
  static AtomType AT_2HD;
  static AtomType AT_3HD;
  static AtomType AT_1HD1;
  static AtomType AT_2HD1;
  static AtomType AT_3HD1;
  static AtomType AT_1HD2;
  static AtomType AT_2HD2;
  static AtomType AT_3HD2;
  static AtomType AT_SD;
  static AtomType AT_OD1;
  static AtomType AT_OD2;
  static AtomType AT_ND1;
  static AtomType AT_ND2;
  static AtomType AT_CE;
  static AtomType AT_CE1;
  static AtomType AT_CE2;
  static AtomType AT_CE3;
  static AtomType AT_HE;
  static AtomType AT_1HE;
  static AtomType AT_2HE;
  static AtomType AT_3HE;
  // static AtomType AT_HE1;
  // static AtomType AT_HE2;
  // static AtomType AT_HE3;
  static AtomType AT_1HE2;
  static AtomType AT_2HE2;
  static AtomType AT_OE1;
  static AtomType AT_OE2;
  static AtomType AT_NE;
  static AtomType AT_NE1;
  static AtomType AT_NE2;
  static AtomType AT_CZ;
  static AtomType AT_CZ2;
  static AtomType AT_CZ3;
  static AtomType AT_NZ;
  static AtomType AT_HZ;
  static AtomType AT_1HZ;
  static AtomType AT_2HZ;
  static AtomType AT_3HZ;
  // static AtomType AT_HZ2;
  // static AtomType AT_HZ3;
  static AtomType AT_CH2;
  static AtomType AT_NH1;
  static AtomType AT_NH2;
  static AtomType AT_OH;
  static AtomType AT_HH;
  static AtomType AT_1HH1;
  static AtomType AT_2HH1;
  static AtomType AT_HH2;
  static AtomType AT_1HH2;
  static AtomType AT_2HH2;
  static AtomType AT_1HH3;
  static AtomType AT_2HH3;
  static AtomType AT_3HH3;
  static AtomType AT_P;
  static AtomType AT_OP1;
  static AtomType AT_OP2;
  static AtomType AT_O5p;
  static AtomType AT_C5p;
  static AtomType AT_H5p;
  static AtomType AT_H5pp;
  static AtomType AT_C4p;
  static AtomType AT_H4p;
  static AtomType AT_O4p;
  static AtomType AT_C1p;
  static AtomType AT_H1p;
  static AtomType AT_C3p;
  static AtomType AT_H3p;
  static AtomType AT_O3p;
  static AtomType AT_C2p;
  static AtomType AT_H2p;
  static AtomType AT_H2pp;
  static AtomType AT_O2p;
  static AtomType AT_HO2p;
  static AtomType AT_N9;
  static AtomType AT_C8;
  static AtomType AT_H8;
  static AtomType AT_N7;
  static AtomType AT_C5;
  static AtomType AT_C4;
  static AtomType AT_N3;
  static AtomType AT_C2;
  static AtomType AT_H2;
  static AtomType AT_N1;
  static AtomType AT_C6;
  static AtomType AT_N6;
  static AtomType AT_H61;
  static AtomType AT_H62;
  static AtomType AT_O6;
  static AtomType AT_H1;
  static AtomType AT_N2;
  static AtomType AT_H21;
  static AtomType AT_H22;
  static AtomType AT_H6;
  static AtomType AT_H5;
  static AtomType AT_O2;
  static AtomType AT_N4;
  static AtomType AT_H41;
  static AtomType AT_H42;
  static AtomType AT_H3;
  static AtomType AT_O4;
  static AtomType AT_C7;
  static AtomType AT_H71;
  static AtomType AT_H72;
  static AtomType AT_H73;

  AtomType get_type() const {
    return AtomType(get_particle()->get_value(type_key_));
  }

  void set_type(AtomType t);

  IMP_DECORATOR_GET_SET_OPT(charge, charge_key_,
                            Float, Float, 0);
  IMP_DECORATOR_GET_SET_OPT(element, element_key_,
                            Int, Int, 0);

  IMP_DECORATOR_GET_SET_OPT(mass, mass_key_,
                            Float, Float, 0);
  /*IMP_DECORATOR_GET_SET_OPT(van_der_waals_radius, vdw_radius_key_,
    Float, Float, 0);*/




};

IMP_OUTPUT_OPERATOR(AtomDecorator);

}

#endif  /* __IMP_ATOM_DECORATOR_H */
