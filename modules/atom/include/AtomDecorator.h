/**
 *  \file AtomDecorator.h     \brief Simple atom decorator.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPATOM_ATOM_DECORATOR_H
#define IMPATOM_ATOM_DECORATOR_H

#include "config.h"
#include "macros.h"
#include "ResidueDecorator.h"
#include "MolecularHierarchyDecorator.h"
#include <IMP/core/utility.h>
#include <IMP/core/XYZDecorator.h>
#include <IMP/core/macros.h>

#include <IMP/base_types.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>

#include <vector>
#include <deque>

IMPATOM_BEGIN_NAMESPACE

IMP_DECLARE_KEY_TYPE(AtomType, IMP_ATOM_TYPE_INDEX);


/** @name PDB Atom Types

    We provide an AtomType instance for each of the standard PDB %atom types.
    These have names such as IMP::atom::AT_N. The full list is elided for
    readability. New types can be added using the
    add_atom_type() function.

    \see AtomDecorator
*/
/*@{*/
/* each static must be on a separate line because of MSVC bug C2487:
   see http://support.microsoft.com/kb/127900/
*/
/** */
IMPATOMEXPORT extern const AtomType AT_UNKNOWN;
#ifndef IMP_DOXYGEN
/** */
IMPATOMEXPORT extern const AtomType AT_N;
/** */
IMPATOMEXPORT extern const AtomType AT_CA;
/** */
IMPATOMEXPORT extern const AtomType AT_C;
/** */
IMPATOMEXPORT extern const AtomType AT_O;
/** */
IMPATOMEXPORT extern const AtomType AT_H;
/** */
IMPATOMEXPORT extern const AtomType AT_1H;
/** */
IMPATOMEXPORT extern const AtomType AT_2H;
/** */
IMPATOMEXPORT extern const AtomType AT_3H;
/** */
IMPATOMEXPORT extern const AtomType AT_HA;
/** */
IMPATOMEXPORT extern const AtomType AT_1HA;
/** */
IMPATOMEXPORT extern const AtomType AT_2HA;
/** */
IMPATOMEXPORT extern const AtomType AT_CB;
/** */
IMPATOMEXPORT extern const AtomType AT_HB;
/** */
IMPATOMEXPORT extern const AtomType AT_1HB;
/** */
IMPATOMEXPORT extern const AtomType AT_2HB;
/** */
IMPATOMEXPORT extern const AtomType AT_3HB;
/** */
IMPATOMEXPORT extern const AtomType AT_OXT;
/** */
IMPATOMEXPORT extern const AtomType AT_CH3;
/** */
IMPATOMEXPORT extern const AtomType AT_CG;
/** */
IMPATOMEXPORT extern const AtomType AT_CG1;
/** */
/** */ IMPATOMEXPORT extern const AtomType AT_CG2;
/** */ IMPATOMEXPORT extern const AtomType AT_HG;
/** */ IMPATOMEXPORT extern const AtomType AT_1HG;
/** */ IMPATOMEXPORT extern const AtomType AT_2HG;
// IMPATOMEXPORT extern const AtomType AT_HG1;
/** */ IMPATOMEXPORT extern const AtomType AT_1HG1;
/** */ IMPATOMEXPORT extern const AtomType AT_2HG1;
/** */ IMPATOMEXPORT extern const AtomType AT_3HG1;
/** */ IMPATOMEXPORT extern const AtomType AT_1HG2;
/** */ IMPATOMEXPORT extern const AtomType AT_2HG2;
/** */ IMPATOMEXPORT extern const AtomType AT_3HG2;
/** */ IMPATOMEXPORT extern const AtomType AT_OG;
/** */ IMPATOMEXPORT extern const AtomType AT_OG1;
/** */ IMPATOMEXPORT extern const AtomType AT_SG;
// IMPATOMEXPORT extern const AtomType AT_HD1;
// IMPATOMEXPORT extern const AtomType AT_HD2;
/** */ IMPATOMEXPORT extern const AtomType AT_CD;
/** */ IMPATOMEXPORT extern const AtomType AT_CD1;
/** */ IMPATOMEXPORT extern const AtomType AT_CD2;
/** */ IMPATOMEXPORT extern const AtomType AT_HD;
/** */ IMPATOMEXPORT extern const AtomType AT_1HD;
/** */ IMPATOMEXPORT extern const AtomType AT_2HD;
/** */ IMPATOMEXPORT extern const AtomType AT_3HD;
/** */ IMPATOMEXPORT extern const AtomType AT_1HD1;
/** */ IMPATOMEXPORT extern const AtomType AT_2HD1;
/** */ IMPATOMEXPORT extern const AtomType AT_3HD1;
/** */ IMPATOMEXPORT extern const AtomType AT_1HD2;
/** */ IMPATOMEXPORT extern const AtomType AT_2HD2;
/** */ IMPATOMEXPORT extern const AtomType AT_3HD2;
/** */ IMPATOMEXPORT extern const AtomType AT_SD;
/** */ IMPATOMEXPORT extern const AtomType AT_OD1;
/** */ IMPATOMEXPORT extern const AtomType AT_OD2;
/** */ IMPATOMEXPORT extern const AtomType AT_ND1;
/** */ IMPATOMEXPORT extern const AtomType AT_ND2;
/** */ IMPATOMEXPORT extern const AtomType AT_CE;
/** */ IMPATOMEXPORT extern const AtomType AT_CE1;
/** */ IMPATOMEXPORT extern const AtomType AT_CE2;
/** */ IMPATOMEXPORT extern const AtomType AT_CE3;
/** */ IMPATOMEXPORT extern const AtomType AT_HE;
/** */ IMPATOMEXPORT extern const AtomType AT_1HE;
/** */ IMPATOMEXPORT extern const AtomType AT_2HE;
/** */ IMPATOMEXPORT extern const AtomType AT_3HE;
// IMPATOMEXPORT extern const AtomType AT_HE1;
// IMPATOMEXPORT extern const AtomType AT_HE2;
// IMPATOMEXPORT extern const AtomType AT_HE3;
/** */ IMPATOMEXPORT extern const AtomType AT_1HE2;
/** */ IMPATOMEXPORT extern const AtomType AT_2HE2;
/** */ IMPATOMEXPORT extern const AtomType AT_OE1;
/** */ IMPATOMEXPORT extern const AtomType AT_OE2;
/** */ IMPATOMEXPORT extern const AtomType AT_NE;
/** */ IMPATOMEXPORT extern const AtomType AT_NE1;
/** */ IMPATOMEXPORT extern const AtomType AT_NE2;
/** */ IMPATOMEXPORT extern const AtomType AT_CZ;
/** */ IMPATOMEXPORT extern const AtomType AT_CZ2;
/** */ IMPATOMEXPORT extern const AtomType AT_CZ3;
/** */ IMPATOMEXPORT extern const AtomType AT_NZ;
/** */ IMPATOMEXPORT extern const AtomType AT_HZ;
/** */ IMPATOMEXPORT extern const AtomType AT_1HZ;
/** */ IMPATOMEXPORT extern const AtomType AT_2HZ;
/** */ IMPATOMEXPORT extern const AtomType AT_3HZ;
// IMPATOMEXPORT extern const AtomType AT_HZ2;
// IMPATOMEXPORT extern const AtomType AT_HZ3;
/** */ IMPATOMEXPORT extern const AtomType AT_CH2;
/** */ IMPATOMEXPORT extern const AtomType AT_NH1;
/** */ IMPATOMEXPORT extern const AtomType AT_NH2;
/** */ IMPATOMEXPORT extern const AtomType AT_OH;
/** */ IMPATOMEXPORT extern const AtomType AT_HH;
/** */ IMPATOMEXPORT extern const AtomType AT_1HH1;
/** */ IMPATOMEXPORT extern const AtomType AT_2HH1;
/** */ IMPATOMEXPORT extern const AtomType AT_HH2;
/** */ IMPATOMEXPORT extern const AtomType AT_1HH2;
/** */ IMPATOMEXPORT extern const AtomType AT_2HH2;
/** */ IMPATOMEXPORT extern const AtomType AT_1HH3;
/** */ IMPATOMEXPORT extern const AtomType AT_2HH3;
/** */ IMPATOMEXPORT extern const AtomType AT_3HH3;
/** */ IMPATOMEXPORT extern const AtomType AT_P;
/** */ IMPATOMEXPORT extern const AtomType AT_OP1;
/** */ IMPATOMEXPORT extern const AtomType AT_OP2;
/** */ IMPATOMEXPORT extern const AtomType AT_O5p;
/** */ IMPATOMEXPORT extern const AtomType AT_C5p;
/** */ IMPATOMEXPORT extern const AtomType AT_H5p;
/** */ IMPATOMEXPORT extern const AtomType AT_H5pp;
/** */ IMPATOMEXPORT extern const AtomType AT_C4p;
/** */ IMPATOMEXPORT extern const AtomType AT_H4p;
/** */ IMPATOMEXPORT extern const AtomType AT_O4p;
/** */ IMPATOMEXPORT extern const AtomType AT_C1p;
/** */ IMPATOMEXPORT extern const AtomType AT_H1p;
/** */ IMPATOMEXPORT extern const AtomType AT_C3p;
/** */ IMPATOMEXPORT extern const AtomType AT_H3p;
/** */ IMPATOMEXPORT extern const AtomType AT_O3p;
/** */ IMPATOMEXPORT extern const AtomType AT_C2p;
/** */ IMPATOMEXPORT extern const AtomType AT_H2p;
/** */ IMPATOMEXPORT extern const AtomType AT_H2pp;
/** */ IMPATOMEXPORT extern const AtomType AT_O2p;
/** */ IMPATOMEXPORT extern const AtomType AT_HO2p;
/** */ IMPATOMEXPORT extern const AtomType AT_N9;
/** */ IMPATOMEXPORT extern const AtomType AT_C8;
/** */ IMPATOMEXPORT extern const AtomType AT_H8;
/** */ IMPATOMEXPORT extern const AtomType AT_N7;
/** */ IMPATOMEXPORT extern const AtomType AT_C5;
/** */ IMPATOMEXPORT extern const AtomType AT_C4;
/** */ IMPATOMEXPORT extern const AtomType AT_N3;
/** */ IMPATOMEXPORT extern const AtomType AT_C2;
/** */ IMPATOMEXPORT extern const AtomType AT_H2;
/** */ IMPATOMEXPORT extern const AtomType AT_N1;
/** */ IMPATOMEXPORT extern const AtomType AT_C6;
/** */ IMPATOMEXPORT extern const AtomType AT_N6;
/** */ IMPATOMEXPORT extern const AtomType AT_H61;
/** */ IMPATOMEXPORT extern const AtomType AT_H62;
/** */ IMPATOMEXPORT extern const AtomType AT_O6;
/** */ IMPATOMEXPORT extern const AtomType AT_H1;
/** */ IMPATOMEXPORT extern const AtomType AT_N2;
/** */ IMPATOMEXPORT extern const AtomType AT_H21;
/** */ IMPATOMEXPORT extern const AtomType AT_H22;
/** */ IMPATOMEXPORT extern const AtomType AT_H6;
/** */ IMPATOMEXPORT extern const AtomType AT_H5;
/** */ IMPATOMEXPORT extern const AtomType AT_O2;
/** */ IMPATOMEXPORT extern const AtomType AT_N4;
/** */ IMPATOMEXPORT extern const AtomType AT_H41;
/** */ IMPATOMEXPORT extern const AtomType AT_H42;
/** */ IMPATOMEXPORT extern const AtomType AT_H3;
/** */ IMPATOMEXPORT extern const AtomType AT_O4;
/** */ IMPATOMEXPORT extern const AtomType AT_C7;
/** */ IMPATOMEXPORT extern const AtomType AT_H71;
/** */ IMPATOMEXPORT extern const AtomType AT_H72;
/** */ IMPATOMEXPORT extern const AtomType AT_H73;
#endif
/*@}*/


//! A decorator for a particle representing an atom.
/**
   \ingroup hierarchy
   \see MolecularHierarchy
 */
class IMPATOMEXPORT AtomDecorator: public core::XYZDecorator,
  public MolecularHierarchyDecorator
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


  IMP_DECORATOR_2(AtomDecorator, IMP::core::XYZDecorator,
                  MolecularHierarchyDecorator)

  Particle* get_particle() const {
    return MolecularHierarchyDecorator::get_particle();
  }

  /** Create a decorator with the passed type and coordinates.*/
  static AtomDecorator create(Particle *p, AtomType t= AT_UNKNOWN,
                              const algebra::Vector3D &v=
                              algebra::Vector3D(0,0,0));

  /** Create a decorator by copying from o.*/
  static AtomDecorator create(Particle *p, AtomDecorator o);

  //! return true if the particle has the needed attributes
  static bool is_instance_of(Particle *p) {
    return p->has_attribute(get_type_key())
      && XYZDecorator::is_instance_of(p)
      && MolecularHierarchyDecorator::is_instance_of(p);
  }

  AtomType get_type() const {
    return AtomType(get_particle()->get_value(get_type_key()));
  }

  void set_type(AtomType t);

  IMP_DECORATOR_GET_SET_OPT(charge, get_charge_key(),
                            Float, Float, 0);
  IMP_DECORATOR_GET_SET_OPT(element, get_element_key(),
                            Int, Int, 0);

  IMP_DECORATOR_GET_SET_OPT(mass, get_mass_key(),
                            Float, Float, 0);
  /** @name The atom index in the input file
      This index is not necessarily unique over any particular
      set of atoms and so should never be used as an atom identifier
      except during I/O.
      @{
  */
  IMP_DECORATOR_GET_SET_OPT(input_index, get_input_index_key(),
                            Int, Int, -1);
  /* @}*/

  //! shows the Atom record in a PDB format
  /** \param index the atom index. If index eq -1 than the original
             atom index (as read from a PDB file) is used.
      \note  The following fileds are currently not displayed:
             chain, residue insertion code, occupancy,  temp. factor
  */
  std::string get_pdb_string(int index=-1);

  /** @name Keys
      These methods provide access to the various keys used to store
      things in the AtomDecorator. These can be used if you want to
      use an attribute to search a set of particles.
      @{
   */
  static IntKey get_type_key();

  static IntKey get_element_key();

  static FloatKey get_mass_key();

  static FloatKey get_charge_key();

  static IntKey get_input_index_key();
  //! @}
};

IMP_OUTPUT_OPERATOR(AtomDecorator);

//! Return the AtomType from the four letter code in the PDB
/** \throw ValueException if nm is invalid.
    \relates AtomDecorator
    \relates AtomType
 */
IMPATOMEXPORT AtomType atom_type_from_pdb_string(std::string nm);


//! Return the index of the residue containing this atom
/** The atom must be part of a molecular hierarchy.
    \relates AtomDecorator
 */
IMPATOMEXPORT int get_residue_index(AtomDecorator d);

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
IMPATOMEXPORT ResidueType get_residue_type(AtomDecorator d);

//! Return the ResidueDecorator containing this atom
/** The atom must be part of a molecular hierarchy.
    \relates AtomDecorator
    \relates ResidueDecorator
    \relates MolecularHierarchyDecorator
 */
IMPATOMEXPORT ResidueDecorator get_residue(AtomDecorator d);

//! Return a particle atom from the residue
/** The residue must be part of a molecular hierarchy.
    \relates AtomDecorator
    \relates ResidueDecorator
    \relates MolecularHierarchyDecorator
 */
IMPATOMEXPORT AtomDecorator get_atom(ResidueDecorator rd, AtomType at);

//! Return the chain id of this atom
/** The atom must be part of a molecular hierarchy.
    \relates AtomDecorator
    \relates ResidueDecorator
    \relates MolecularHierarchyDecorator
 */
IMPATOMEXPORT char get_chain(AtomDecorator d);


//! Create a new AtomType
/** This creates a new AtomType (returned) and sets up the mapping
    between the AtomType and the proper element.
    \note This method has not been tested. If you use it, please
    write a test and remove this comment.
    \relates AtomType
*/
IMPATOMEXPORT AtomType add_atom_type(std::string name,
                                    AtomDecorator::Element element);


IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_ATOM_DECORATOR_H */
