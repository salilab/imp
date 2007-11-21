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
  static AtomType AT_UNKNOWN, AT_N, AT_CA, AT_C, AT_O,
    AT_H, AT_1H, AT_2H, AT_3H,
    AT_HA, AT_1HA, AT_2HA,
    AT_CB, AT_HB, AT_1HB, AT_2HB, AT_3HB,
    AT_OXT, AT_CH3,
    AT_CG, AT_CG1, AT_CG2, AT_HG, AT_1HG, AT_2HG, //AT_HG1,
    AT_1HG1, AT_2HG1, AT_3HG1, AT_1HG2,
    AT_2HG2, AT_3HG2, AT_OG, AT_OG1, AT_SG,
    // AT_HD1, AT_HD2,
    AT_CD, AT_CD1, AT_CD2,  AT_HD, AT_1HD, AT_2HD, AT_3HD, AT_1HD1,
    AT_2HD1, AT_3HD1, AT_1HD2, AT_2HD2, AT_3HD2, AT_SD,
    AT_OD1, AT_OD2, AT_ND1, AT_ND2,
    AT_CE, AT_CE1, AT_CE2, AT_CE3, AT_HE, AT_1HE, AT_2HE,
    AT_3HE, //AT_HE1, AT_HE2, AT_HE3,
    AT_1HE2, AT_2HE2,
    AT_OE1, AT_OE2, AT_NE, AT_NE1, AT_NE2,
    AT_CZ, AT_CZ2, AT_CZ3, AT_NZ, AT_HZ, AT_1HZ, AT_2HZ,
    AT_3HZ, // AT_HZ2, AT_HZ3,
    AT_CH2, AT_NH1, AT_NH2, AT_OH, AT_HH, AT_1HH1,
    AT_2HH1, AT_HH2, AT_1HH2, AT_2HH2,
    AT_1HH3, AT_2HH3, AT_3HH3,
    AT_P, AT_OP1, AT_OP2,
    AT_O5p, AT_C5p, AT_H5p, AT_H5pp,
    AT_C4p, AT_H4p, AT_O4p, AT_C1p, AT_H1p,
    AT_C3p, AT_H3p, AT_O3p, AT_C2p, AT_H2p,
    AT_H2pp, AT_O2p, AT_HO2p,
    AT_N9, AT_C8, AT_H8,
    AT_N7, AT_C5, AT_C4, AT_N3,
    AT_C2, AT_H2, AT_N1, AT_C6, AT_N6,
    AT_H61, AT_H62,
    AT_O6, AT_H1, AT_N2, AT_H21, AT_H22,
    AT_H6, AT_H5, AT_O2, AT_N4, AT_H41, AT_H42,
    AT_H3, AT_O4, AT_C7, AT_H71, AT_H72, AT_H73;

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
