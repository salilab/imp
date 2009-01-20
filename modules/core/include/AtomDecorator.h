/**
 *  \file AtomDecorator.h     \brief Simple atom decorator.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_ATOM_DECORATOR_H
#define IMPCORE_ATOM_DECORATOR_H

#include "config.h"
#include "utility.h"
#include "XYZDecorator.h"
#include "macros.h"
#include "ResidueDecorator.h"

#include <IMP/base_types.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>

#include <vector>
#include <deque>

IMPCORE_BEGIN_NAMESPACE

IMP_DECLARE_KEY_TYPE(AtomType, IMP_ATOM_TYPE_INDEX);


/** @name The standard PDB atom types.
    \relates AtomDecorator
*/
/*@{*/
/* each static must be on a separate line because of MSVC bug C2487:
   see http://support.microsoft.com/kb/127900/
*/
/** */
IMPCOREEXPORT extern const AtomType AT_UNKNOWN;
/** */
IMPCOREEXPORT extern const AtomType AT_N;
/** */
IMPCOREEXPORT extern const AtomType AT_CA;
/** */
IMPCOREEXPORT extern const AtomType AT_C;
/** */
IMPCOREEXPORT extern const AtomType AT_O;
/** */
IMPCOREEXPORT extern const AtomType AT_H;
/** */
IMPCOREEXPORT extern const AtomType AT_1H;
/** */
IMPCOREEXPORT extern const AtomType AT_2H;
/** */
IMPCOREEXPORT extern const AtomType AT_3H;
/** */
IMPCOREEXPORT extern const AtomType AT_HA;
/** */
IMPCOREEXPORT extern const AtomType AT_1HA;
/** */
IMPCOREEXPORT extern const AtomType AT_2HA;
/** */
IMPCOREEXPORT extern const AtomType AT_CB;
/** */
IMPCOREEXPORT extern const AtomType AT_HB;
/** */
IMPCOREEXPORT extern const AtomType AT_1HB;
/** */
IMPCOREEXPORT extern const AtomType AT_2HB;
/** */
IMPCOREEXPORT extern const AtomType AT_3HB;
/** */
IMPCOREEXPORT extern const AtomType AT_OXT;
/** */
IMPCOREEXPORT extern const AtomType AT_CH3;
/** */
IMPCOREEXPORT extern const AtomType AT_CG;
/** */
IMPCOREEXPORT extern const AtomType AT_CG1;
/** */
/** */ IMPCOREEXPORT extern const AtomType AT_CG2;
/** */ IMPCOREEXPORT extern const AtomType AT_HG;
/** */ IMPCOREEXPORT extern const AtomType AT_1HG;
/** */ IMPCOREEXPORT extern const AtomType AT_2HG;
// IMPCOREEXPORT extern const AtomType AT_HG1;
/** */ IMPCOREEXPORT extern const AtomType AT_1HG1;
/** */ IMPCOREEXPORT extern const AtomType AT_2HG1;
/** */ IMPCOREEXPORT extern const AtomType AT_3HG1;
/** */ IMPCOREEXPORT extern const AtomType AT_1HG2;
/** */ IMPCOREEXPORT extern const AtomType AT_2HG2;
/** */ IMPCOREEXPORT extern const AtomType AT_3HG2;
/** */ IMPCOREEXPORT extern const AtomType AT_OG;
/** */ IMPCOREEXPORT extern const AtomType AT_OG1;
/** */ IMPCOREEXPORT extern const AtomType AT_SG;
// IMPCOREEXPORT extern const AtomType AT_HD1;
// IMPCOREEXPORT extern const AtomType AT_HD2;
/** */ IMPCOREEXPORT extern const AtomType AT_CD;
/** */ IMPCOREEXPORT extern const AtomType AT_CD1;
/** */ IMPCOREEXPORT extern const AtomType AT_CD2;
/** */ IMPCOREEXPORT extern const AtomType AT_HD;
/** */ IMPCOREEXPORT extern const AtomType AT_1HD;
/** */ IMPCOREEXPORT extern const AtomType AT_2HD;
/** */ IMPCOREEXPORT extern const AtomType AT_3HD;
/** */ IMPCOREEXPORT extern const AtomType AT_1HD1;
/** */ IMPCOREEXPORT extern const AtomType AT_2HD1;
/** */ IMPCOREEXPORT extern const AtomType AT_3HD1;
/** */ IMPCOREEXPORT extern const AtomType AT_1HD2;
/** */ IMPCOREEXPORT extern const AtomType AT_2HD2;
/** */ IMPCOREEXPORT extern const AtomType AT_3HD2;
/** */ IMPCOREEXPORT extern const AtomType AT_SD;
/** */ IMPCOREEXPORT extern const AtomType AT_OD1;
/** */ IMPCOREEXPORT extern const AtomType AT_OD2;
/** */ IMPCOREEXPORT extern const AtomType AT_ND1;
/** */ IMPCOREEXPORT extern const AtomType AT_ND2;
/** */ IMPCOREEXPORT extern const AtomType AT_CE;
/** */ IMPCOREEXPORT extern const AtomType AT_CE1;
/** */ IMPCOREEXPORT extern const AtomType AT_CE2;
/** */ IMPCOREEXPORT extern const AtomType AT_CE3;
/** */ IMPCOREEXPORT extern const AtomType AT_HE;
/** */ IMPCOREEXPORT extern const AtomType AT_1HE;
/** */ IMPCOREEXPORT extern const AtomType AT_2HE;
/** */ IMPCOREEXPORT extern const AtomType AT_3HE;
// IMPCOREEXPORT extern const AtomType AT_HE1;
// IMPCOREEXPORT extern const AtomType AT_HE2;
// IMPCOREEXPORT extern const AtomType AT_HE3;
/** */ IMPCOREEXPORT extern const AtomType AT_1HE2;
/** */ IMPCOREEXPORT extern const AtomType AT_2HE2;
/** */ IMPCOREEXPORT extern const AtomType AT_OE1;
/** */ IMPCOREEXPORT extern const AtomType AT_OE2;
/** */ IMPCOREEXPORT extern const AtomType AT_NE;
/** */ IMPCOREEXPORT extern const AtomType AT_NE1;
/** */ IMPCOREEXPORT extern const AtomType AT_NE2;
/** */ IMPCOREEXPORT extern const AtomType AT_CZ;
/** */ IMPCOREEXPORT extern const AtomType AT_CZ2;
/** */ IMPCOREEXPORT extern const AtomType AT_CZ3;
/** */ IMPCOREEXPORT extern const AtomType AT_NZ;
/** */ IMPCOREEXPORT extern const AtomType AT_HZ;
/** */ IMPCOREEXPORT extern const AtomType AT_1HZ;
/** */ IMPCOREEXPORT extern const AtomType AT_2HZ;
/** */ IMPCOREEXPORT extern const AtomType AT_3HZ;
// IMPCOREEXPORT extern const AtomType AT_HZ2;
// IMPCOREEXPORT extern const AtomType AT_HZ3;
/** */ IMPCOREEXPORT extern const AtomType AT_CH2;
/** */ IMPCOREEXPORT extern const AtomType AT_NH1;
/** */ IMPCOREEXPORT extern const AtomType AT_NH2;
/** */ IMPCOREEXPORT extern const AtomType AT_OH;
/** */ IMPCOREEXPORT extern const AtomType AT_HH;
/** */ IMPCOREEXPORT extern const AtomType AT_1HH1;
/** */ IMPCOREEXPORT extern const AtomType AT_2HH1;
/** */ IMPCOREEXPORT extern const AtomType AT_HH2;
/** */ IMPCOREEXPORT extern const AtomType AT_1HH2;
/** */ IMPCOREEXPORT extern const AtomType AT_2HH2;
/** */ IMPCOREEXPORT extern const AtomType AT_1HH3;
/** */ IMPCOREEXPORT extern const AtomType AT_2HH3;
/** */ IMPCOREEXPORT extern const AtomType AT_3HH3;
/** */ IMPCOREEXPORT extern const AtomType AT_P;
/** */ IMPCOREEXPORT extern const AtomType AT_OP1;
/** */ IMPCOREEXPORT extern const AtomType AT_OP2;
/** */ IMPCOREEXPORT extern const AtomType AT_O5p;
/** */ IMPCOREEXPORT extern const AtomType AT_C5p;
/** */ IMPCOREEXPORT extern const AtomType AT_H5p;
/** */ IMPCOREEXPORT extern const AtomType AT_H5pp;
/** */ IMPCOREEXPORT extern const AtomType AT_C4p;
/** */ IMPCOREEXPORT extern const AtomType AT_H4p;
/** */ IMPCOREEXPORT extern const AtomType AT_O4p;
/** */ IMPCOREEXPORT extern const AtomType AT_C1p;
/** */ IMPCOREEXPORT extern const AtomType AT_H1p;
/** */ IMPCOREEXPORT extern const AtomType AT_C3p;
/** */ IMPCOREEXPORT extern const AtomType AT_H3p;
/** */ IMPCOREEXPORT extern const AtomType AT_O3p;
/** */ IMPCOREEXPORT extern const AtomType AT_C2p;
/** */ IMPCOREEXPORT extern const AtomType AT_H2p;
/** */ IMPCOREEXPORT extern const AtomType AT_H2pp;
/** */ IMPCOREEXPORT extern const AtomType AT_O2p;
/** */ IMPCOREEXPORT extern const AtomType AT_HO2p;
/** */ IMPCOREEXPORT extern const AtomType AT_N9;
/** */ IMPCOREEXPORT extern const AtomType AT_C8;
/** */ IMPCOREEXPORT extern const AtomType AT_H8;
/** */ IMPCOREEXPORT extern const AtomType AT_N7;
/** */ IMPCOREEXPORT extern const AtomType AT_C5;
/** */ IMPCOREEXPORT extern const AtomType AT_C4;
/** */ IMPCOREEXPORT extern const AtomType AT_N3;
/** */ IMPCOREEXPORT extern const AtomType AT_C2;
/** */ IMPCOREEXPORT extern const AtomType AT_H2;
/** */ IMPCOREEXPORT extern const AtomType AT_N1;
/** */ IMPCOREEXPORT extern const AtomType AT_C6;
/** */ IMPCOREEXPORT extern const AtomType AT_N6;
/** */ IMPCOREEXPORT extern const AtomType AT_H61;
/** */ IMPCOREEXPORT extern const AtomType AT_H62;
/** */ IMPCOREEXPORT extern const AtomType AT_O6;
/** */ IMPCOREEXPORT extern const AtomType AT_H1;
/** */ IMPCOREEXPORT extern const AtomType AT_N2;
/** */ IMPCOREEXPORT extern const AtomType AT_H21;
/** */ IMPCOREEXPORT extern const AtomType AT_H22;
/** */ IMPCOREEXPORT extern const AtomType AT_H6;
/** */ IMPCOREEXPORT extern const AtomType AT_H5;
/** */ IMPCOREEXPORT extern const AtomType AT_O2;
/** */ IMPCOREEXPORT extern const AtomType AT_N4;
/** */ IMPCOREEXPORT extern const AtomType AT_H41;
/** */ IMPCOREEXPORT extern const AtomType AT_H42;
/** */ IMPCOREEXPORT extern const AtomType AT_H3;
/** */ IMPCOREEXPORT extern const AtomType AT_O4;
/** */ IMPCOREEXPORT extern const AtomType AT_C7;
/** */ IMPCOREEXPORT extern const AtomType AT_H71;
/** */ IMPCOREEXPORT extern const AtomType AT_H72;
/** */ IMPCOREEXPORT extern const AtomType AT_H73;
/*@}*/


//! A decorator for a particle representing an atom.
/**
   At some point the Types and elements need to be expanded to be more complete.
   Elements can be done exhaustively, but we need some sort of dynamic
   mechanism for the types. This could be done by having an
   add_type(string) method which adds a string name and assigns it to the
   next unused type int available.
   \ingroup hierarchy
   \ingroup decorators
 */
class IMPCOREEXPORT AtomDecorator: public XYZDecorator
{
public:
  //! The various elements currently supported
  /**
     The enum just provides mapping from a name to an atomic number.
   */
  enum Element {UNKNOWN_ELEMENT=0, OH=-1, H2O=-2,
                H= 1, He= 2, Li= 3, Be= 4, B= 5, C= 6, N= 7,
                O= 8, F= 9, Ne= 10, Na= 11, Mg= 12, Al= 13, Si= 14,
                P= 15, S= 16, Cl= 17, Ar= 18, K= 19, Ca= 20, Sc= 21,
                Ti= 22, V= 23, Cr= 24, Mn= 25, Fe= 26, Co= 27, Ni= 28,
                Cu= 29, Zn= 30, Ga= 31, Ge= 32, As= 33, Se= 34, Br= 35,
                Kr= 36, Rb= 37, Sr= 38, Y= 39, Zr= 40, Nb= 41, Mo= 42,
                Tc= 43, Ru= 44, Rh= 45, Pd= 46, Ag= 47, Cd= 48, In= 49,
                Sn= 50, Sb= 51, Te= 52, I= 53, Xe= 54, Cs= 55, Ba= 56,
                La= 57, Ce= 58, Pr= 59, Nd= 60, Pm= 61, Sm= 62, Eu= 63,
                Gd= 64, Tb= 65, Dy= 66, Ho= 67, Er= 68, Tm= 69, Yb= 70,
                Lu= 71, Hf= 72, Ta= 73, W= 74, Re= 75, Os= 76, Ir= 77,
                Pt= 78, Au= 79, Hg= 80, Tl= 81, Pb= 82, Bi= 83, Po= 84,
                At= 85, Rn= 86, Fr= 87, Ra= 88, Ac= 89, Th= 90, Pa= 91,
                U= 92, Np= 93, Pu= 94, Am= 95, Cm= 96, Bk= 97, Cf= 98,
                Es= 99, Fm= 100, Md= 101, No= 102, Lr= 103, Db= 104, Jl= 105,
                Rf= 106
  };


  IMP_DECORATOR(AtomDecorator, XYZDecorator)

  /** Create a decorator with the passed type and coordinates.*/
  static AtomDecorator create(Particle *p, AtomType t= AT_UNKNOWN,
                              const Vector3D &v= Vector3D(0,0,0)) {
    p->add_attribute(get_type_key(), t.get_index());
    XYZDecorator::create(p, v);
    return AtomDecorator(p);
  }

  //! return true if the particle has the needed attributes
  static bool is_instance_of(Particle *p) {
    return p->has_attribute(get_type_key())
      && XYZDecorator::is_instance_of(p);
  }

  /** get the stored type */
  AtomType get_type() const {
    return AtomType(get_particle()->get_value(get_type_key()));
  }

  /** set the stored type */
  void set_type(AtomType t);

  IMP_DECORATOR_GET_SET_OPT(charge, get_charge_key(),
                            Float, Float, 0);
  IMP_DECORATOR_GET_SET_OPT(element, get_element_key(),
                            Int, Int, 0);

  IMP_DECORATOR_GET_SET_OPT(mass, get_mass_key(),
                            Float, Float, 0);

  /** Get the key storing the type */
  static IntKey get_type_key();

  /** Get the key storing the element */
  static IntKey get_element_key();

  /** Get the key storing the mass */
  static FloatKey get_mass_key();

  /** Get the key storing the mass */
  static FloatKey get_charge_key();
};

IMP_OUTPUT_OPERATOR(AtomDecorator);

//! Return the AtomType from the four letter code in the PDB
/** \throw ValueException if nm is invalid.
    \relates AtomDecorator
    \relates AtomType
 */
IMPCOREEXPORT AtomType atom_type_from_pdb_string(std::string nm);


//! Return the index of the residue containing this atom
/** The atom must be part of a molecular hierarchy.
 */
IMPCOREEXPORT unsigned int get_residue_index(AtomDecorator d);

#ifdef SWIG
// ResidueType is a typedef so this is invalid C++ code, but swig needs it
class ResidueType;
class ResidueDecorator;
#endif

//! Return the type of the residue containing this atom
/** The atom must be part of a molecular hierarchy.
    \relates AtomDecorator
    \relates ResidueDecorator
    \relates MolecularHierarchyDecorator
 */
IMPCOREEXPORT ResidueType get_residue_type(AtomDecorator d);

//! Return the ResidueDecorator containing this atom
/** The atom must be part of a molecular hierarchy.
    \relates AtomDecorator
    \relates ResidueDecorator
    \relates MolecularHierarchyDecorator
 */
IMPCOREEXPORT ResidueDecorator get_residue(AtomDecorator d);

//! Return a particle atom from the residue
/** The residue must be part of a molecular hierarchy.
    \relates AtomDecorator
    \relates ResidueDecorator
    \relates MolecularHierarchyDecorator
 */
IMPCOREEXPORT AtomDecorator get_atom(ResidueDecorator rd, AtomType at);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_ATOM_DECORATOR_H */
